#include <algorithm>
#include "d2tmh.h"

#include "cMouseUnitsSelectedState.h"

cMouseUnitsSelectedState::cMouseUnitsSelectedState(cPlayer *player, cGameControlsContext *context, cMouse *mouse) :
        cMouseState(player, context, mouse),
        selectedUnits(),
        harvestersSelected(false),
        infantrySelected(false),
        repairableUnitsSelected(false),
        state(SELECTED_STATE_MOVE),
        prevState(SELECTED_STATE_MOVE) {

}

void cMouseUnitsSelectedState::onNotifyMouseEvent(const s_MouseEvent &event) {

    // these methods can have a side-effect which changes mouseTile...
    switch (event.eventType) {
        case MOUSE_LEFT_BUTTON_PRESSED:
            mouse->boxSelectLogic(context->getMouseCell());
            break;
        case MOUSE_LEFT_BUTTON_CLICKED:
            onMouseLeftButtonClicked(event);
            break;
        case MOUSE_RIGHT_BUTTON_PRESSED:
            onMouseRightButtonPressed(event);
            break;
        case MOUSE_RIGHT_BUTTON_CLICKED:
            onMouseRightButtonClicked(event);
            break;
        case MOUSE_MOVED_TO:
            onMouseMovedTo(event);
            break;
        default:
            break;
    }

    // ... so set it here
    if (context->isState(MOUSESTATE_UNITS_SELECTED)) { // if , required in case we switched state
        mouse->setTile(mouseTile);
    }
}

cMouseUnitsSelectedState::~cMouseUnitsSelectedState() {

}

void cMouseUnitsSelectedState::onMouseLeftButtonClicked(const s_MouseEvent &event) {
    if (mouse->isBoxSelecting()) {
        // clear only when we don't want to add to selection
        if (state != SELECTED_STATE_ADD_TO_SELECTION) {
            player->deselectAllUnits();
            selectedUnits.clear();
        }

        bool infantryReporting = false;
        bool unitReporting = false;

        // remember, these are screen coordinates
        // TODO: Make it use absolute coordinates? (so we could have a rectangle bigger than the screen at one point?)
        cRectangle boxSelectRectangle = mouse->getBoxSelectRectangle();

        const std::vector<int> &ids = player->getAllMyUnitsWithinViewportRect(boxSelectRectangle);

        // check if there is a harvester in this group
        auto position = std::find_if(ids.begin(), ids.end(), [&](const int &id) { return unit[id].isHarvester(); });
        bool hasHarvesterSelected = position != ids.end();

        position = std::find_if(ids.begin(), ids.end(),
                                [&](const int &id) { return !unit[id].isHarvester() && !unit[id].isAirbornUnit(); });
        bool nonAirbornNonHarvesterUnitSelected = position != ids.end();

        if (hasHarvesterSelected && !nonAirbornNonHarvesterUnitSelected) {
            // select all the harvester units, skip airborn
            for (auto id: ids) {
                cUnit &pUnit = unit[id];
                if (pUnit.isAirbornUnit()) continue;
                if (!pUnit.isHarvester()) continue;
                pUnit.bSelected = true;
                unitReporting = true; // do it here, instead of iterating again
            }
        } else {
            // select all the non-harvester,non-airborn units
            for (auto id: ids) {
                cUnit &pUnit = unit[id];
                if (pUnit.isAirbornUnit()) continue;
                if (pUnit.isHarvester()) continue;
                pUnit.bSelected = true;
                if (pUnit.isInfantryUnit()) {
                    infantryReporting = true;
                } else {
                    unitReporting = true;
                }
            }
        }

        if (unitReporting) {
            play_sound_id(SOUND_REPORTING);
        }

        if (infantryReporting) {
            play_sound_id(SOUND_YESSIR);
        }

        selectedUnits = player->getSelectedUnits();
        if (selectedUnits.empty()) {
            context->setMouseState(MOUSESTATE_SELECT);
        }
    } else {
        // single click, no box select
        evaluateSelectedUnits();

        int mouseCell = context->getMouseCell();

        bool infantryAcknowledged = false;
        bool unitAcknowledged = false;
        if (state == SELECTED_STATE_SELECT) {
            int hoverStructureId = context->getIdOfStructureWhereMouseHovers();
            if (hoverStructureId > -1) {
                player->selected_structure = hoverStructureId;
                cAbstractStructure *pStructure = player->getSelectedStructure();
                if (pStructure && pStructure->isValid() && pStructure->belongsTo(player)) {
                    player->getSideBar()->setSelectedListId(pStructure->getAssociatedListID());
                } else {
                    player->selected_structure = -1;
                }
            }
        } else  if (state == SELECTED_STATE_REPAIR ||
            state == SELECTED_STATE_REFINERY ||
            state == SELECTED_STATE_MOVE) {

            for (auto id: selectedUnits) {
                cUnit &pUnit = unit[id];
                if (state == SELECTED_STATE_REPAIR) {
                    // only send units that are eligible for repair to facility
                    if (pUnit.isEligibleForRepair()) {
                        pUnit.move_to(mouseCell);
                    }
                    unitAcknowledged = true;
                } else if (state == SELECTED_STATE_REFINERY) {
                    // only send harvesters in group
                    if (pUnit.isHarvester()) {
                        pUnit.move_to(mouseCell);
                    }
                    unitAcknowledged = true;
                } else {
                    if (pUnit.isInfantryUnit()) {
                        infantryAcknowledged = true;
                    }
                    unitAcknowledged = true;
                    pUnit.move_to(mouseCell);
                }
            }
            spawnParticle(D2TM_PARTICLE_MOVE);
        } else if (state == SELECTED_STATE_ATTACK || state == SELECTED_STATE_FORCE_ATTACK) {
            for (auto id: selectedUnits) {
                cUnit &pUnit = unit[id];
                if (pUnit.isHarvester()) {
                    continue;
                }
                if (pUnit.isInfantryUnit()) {
                    infantryAcknowledged = true;
                } else {
                    unitAcknowledged = true;
                }
                pUnit.attackAt(mouseCell);
            }

            spawnParticle(D2TM_PARTICLE_ATTACK);
        }

        if (infantryAcknowledged) {
            play_sound_id(SOUND_MOVINGOUT + rnd(2));
        }

        if (unitAcknowledged) {
            play_sound_id(SOUND_ACKNOWLEDGED + rnd(3));
        }
    }

    mouse->resetBoxSelect();
}

void cMouseUnitsSelectedState::onMouseRightButtonPressed(const s_MouseEvent &event) {
    mouse->dragViewportInteraction();
}

void cMouseUnitsSelectedState::onMouseRightButtonClicked(const s_MouseEvent &event) {
    // if we were dragging the viewport, keep the units and this state.
    if (!mouse->isMapScrolling()) {
        player->deselectAllUnits();
        // back to "select" state
        context->setMouseState(MOUSESTATE_SELECT);
    }

    mouse->resetDragViewportInteraction();
}

void cMouseUnitsSelectedState::onMouseMovedTo(const s_MouseEvent &event) {
    if (state == SELECTED_STATE_FORCE_ATTACK) {
        mouseTile = MOUSE_ATTACK;
    } if (state == SELECTED_STATE_ADD_TO_SELECTION) {
        mouseTile = MOUSE_NORMAL;
    } else {
        evaluateMouseMoveState();
    }
}

void cMouseUnitsSelectedState::evaluateMouseMoveState() {
    mouseTile = MOUSE_MOVE;
    setState(SELECTED_STATE_MOVE);

    cAbstractStructure *hoverStructure = context->getStructurePointerWhereMouseHovers();

    // this feels a little awkward, but having an extra 'bool' for this if statement
// is probably a bit too much at this point.
    bool unitsWhichCanAttackSelected = infantrySelected || repairableUnitsSelected; // don't try to attack with harvesters

    if (hoverStructure) {
        if (!hoverStructure->getPlayer()->isSameTeamAs(player)) {
            if (unitsWhichCanAttackSelected) {
                mouseTile = MOUSE_ATTACK;
                setState(SELECTED_STATE_ATTACK);
            }
        } else if (hoverStructure->belongsTo(player)) {
            if (hoverStructure->getType() == REFINERY) {
                if (harvestersSelected) {
                    setState(SELECTED_STATE_REFINERY);
                } else {
                    setState(SELECTED_STATE_SELECT);
                    mouseTile = MOUSE_NORMAL; // allow "selecting" of structure, event though we have units selected
                }
            } else if (hoverStructure->getType() == REPAIR) {
                if (repairableUnitsSelected) {
                    setState(SELECTED_STATE_REPAIR);
                } else {
                    setState(SELECTED_STATE_SELECT);
                    mouseTile = MOUSE_NORMAL; // allow "selecting" of structure, event though we have units selected
                }
            } else {
                setState(SELECTED_STATE_SELECT);
                mouseTile = MOUSE_NORMAL; // allow "selecting" of structure, event though we have units selected
            }
        }
    }

    if (unitsWhichCanAttackSelected) {
        int hoverUnitId = context->getIdOfUnitWhereMouseHovers();
        if (hoverUnitId > -1) {
            if (unit[hoverUnitId].isValid() && !unit[hoverUnitId].getPlayer()->isSameTeamAs(player)) {
                mouseTile = MOUSE_ATTACK;
                setState(SELECTED_STATE_ATTACK);
            }
        }
    }
}

void cMouseUnitsSelectedState::onStateSet() {
    // only on state change we refresh the entire vector
    selectedUnits = player->getSelectedUnits();

    evaluateSelectedUnits();
    updateSelectedUnitsState();

    mouseTile = MOUSE_MOVE; // TODO: check if unit at cell
    mouse->setTile(mouseTile);
}

void cMouseUnitsSelectedState::updateSelectedUnitsState() {
    harvestersSelected = false;
    infantrySelected = false;
    repairableUnitsSelected = false;
    for (auto id: selectedUnits) {
        cUnit &pUnit = unit[id];
        if (pUnit.isHarvester()) {
            harvestersSelected = true;
            repairableUnitsSelected = true;
        } else if (pUnit.isInfantryUnit()) {
            infantrySelected = true;
        } else {
            repairableUnitsSelected = true;
        }

        if (harvestersSelected && infantrySelected && repairableUnitsSelected) {
            // no need to evaluate further
            break;
        }
    }
}

void cMouseUnitsSelectedState::evaluateSelectedUnits() {
    // remove all invalid and units that (no longer) belong to us (ie, deviated?)
    // TODO: let this (also?) react to game events, so we remove them when it happens.
    selectedUnits.erase(
            std::remove_if(
                    selectedUnits.begin(),
                    selectedUnits.end(),
                    [this](int id) {
                        return !unit[id].isValid() || // no (longer) valid
                               !unit[id].belongsTo(player) || // no longer belongs to player
                               unit[id].iTempHitPoints > -1; // hidden (entered structure, etc). Forget it then.
                    }),
            selectedUnits.end()
    );
}

void cMouseUnitsSelectedState::setState(eMouseUnitsSelectedState newState) {
    if (newState != state) {
        char msg[255];
        sprintf(msg, "cMouseUnitsSelectedState: Changed state from [%s] to [%s]", mouseUnitsSelectedStateString(state),
                mouseUnitsSelectedStateString(newState));
        logbook(msg);
        this->prevState = this->state;
        this->state = newState;
    }
}

void cMouseUnitsSelectedState::onNotifyKeyboardEvent(const s_KeyboardEvent &event) {
    switch (event.eventType) {
        case KEY_HOLD:
            onKeyDown(event);
            break;
        case KEY_PRESSED:
            onKeyPressed(event);
            break;
        default:
            break;

    }

    if (state == SELECTED_STATE_MOVE) {

    }

    if (context->isState(MOUSESTATE_UNITS_SELECTED)) { // if , required in case we switched state
        mouse->setTile(mouseTile);
    }
}

void cMouseUnitsSelectedState::onKeyDown(const s_KeyboardEvent &event) {
    if (event.key == KEY_LCONTROL || event.key == KEY_RCONTROL) {
        setState(SELECTED_STATE_ATTACK);
        mouseTile = MOUSE_ATTACK;
    }

    if (event.key == KEY_LSHIFT || event.key == KEY_RSHIFT) {
        setState(SELECTED_STATE_ADD_TO_SELECTION);
        mouseTile = MOUSE_NORMAL;
        // don't change mouse-tile
    }

    // force move?
}

void cMouseUnitsSelectedState::onKeyPressed(const s_KeyboardEvent &event) {
    if (event.key == KEY_LCONTROL || event.key == KEY_RCONTROL) {
        setState(SELECTED_STATE_ATTACK);
        mouseTile = MOUSE_ATTACK;
    }

    if (event.key == KEY_LSHIFT || event.key == KEY_RSHIFT) {
        toPreviousState();
        evaluateMouseMoveState();
    }


    // go to repair state
    if (event.key == KEY_R) {
        context->setMouseState(MOUSESTATE_REPAIR);
    }

    // force move?
}

void cMouseUnitsSelectedState::toPreviousState() {
    state = prevState;
}

void cMouseUnitsSelectedState::spawnParticle(const int type) {
    int absoluteXCoordinate = mapCamera->getAbsMapMouseX(mouse_x);
    int absoluteYCoordinate = mapCamera->getAbsMapMouseY(mouse_y);
    cParticle::create(absoluteXCoordinate, absoluteYCoordinate, type, -1, -1);
}
