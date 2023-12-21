#include "cMouseUnitsSelectedState.h"

#include "d2tmc.h"
#include "data/gfxdata.h"
#include "controls/cGameControlsContext.h"
#include "gameobjects/particles/cParticle.h"
#include "map/cMapCamera.h"
#include "player/cPlayer.h"
#include "utils/cSoundPlayer.h"

#include <fmt/core.h>
#include <allegro/mouse.h>

#include <algorithm>
#include <string>

namespace {

std::string mouseUnitsSelectedStateString(eMouseUnitsSelectedState state) {
    switch (state) {
        case SELECTED_STATE_MOVE: return "SELECTED_STATE_MOVE";
        case SELECTED_STATE_SELECT: return "SELECTED_STATE_SELECT";
        case SELECTED_STATE_ADD_TO_SELECTION: return "SELECTED_STATE_ADD_TO_SELECTION";
        case SELECTED_STATE_ATTACK: return "SELECTED_STATE_ATTACK";
        case SELECTED_STATE_FORCE_ATTACK: return "SELECTED_STATE_FORCE_ATTACK";
        case SELECTED_STATE_REPAIR: return "SELECTED_STATE_REPAIR";
        case SELECTED_STATE_CAPTURE: return "SELECTED_STATE_CAPTURE";
        case SELECTED_STATE_REFINERY: return "SELECTED_STATE_REFINERY";
        default:
            assert(false);
            break;
    }
    return {};
}

}

cMouseUnitsSelectedState::cMouseUnitsSelectedState(cPlayer *player, cGameControlsContext *context, cMouse *mouse) :
        cMouseState(player, context, mouse),
        m_harvestersSelected(false),
        m_infantrySelected(false),
        m_infantryShouldCapture(false),
        m_repairableUnitsSelected(false),
        m_state(SELECTED_STATE_MOVE),
        m_prevState(SELECTED_STATE_MOVE) {

}

void cMouseUnitsSelectedState::onNotifyGameEvent(const s_GameEvent &event) {
    logbook(fmt::format(
        "cMouseUnitsSelectedState::onNotifyGameEvent(): entityType:{}, eventType:{}",
        event.entityType, event.eventType
    ));

    if (event.entityType == eBuildType::UNIT) {
        if (event.eventType == eGameEventType::GAME_EVENT_DESTROYED) {
            deselectUnit(event.entityID);
        }
    }
}

void cMouseUnitsSelectedState::onNotifyMouseEvent(const s_MouseEvent &event) {

    // these methods can have a side-effect which changes mouseTile...
    switch (event.eventType) {
        case MOUSE_LEFT_BUTTON_PRESSED:
            m_mouse->boxSelectLogic(m_context->getMouseCell());
            break;
        case MOUSE_LEFT_BUTTON_CLICKED:
            onMouseLeftButtonClicked();
            break;
        case MOUSE_RIGHT_BUTTON_PRESSED:
            onMouseRightButtonPressed();
            break;
        case MOUSE_RIGHT_BUTTON_CLICKED:
            onMouseRightButtonClicked();
            break;
        case MOUSE_MIDDLE_BUTTON_CLICKED:
            onMouseMiddleButtonClicked();
            break;
        case MOUSE_MOVED_TO:
            onMouseMovedTo();
            break;
        default:
            break;
    }

    // ... so set it here
    if (m_context->isState(MOUSESTATE_UNITS_SELECTED)) { // if , required in case we switched state
        m_mouse->setTile(mouseTile);
    }
}

bool cMouseUnitsSelectedState::deselectUnit(int id) {
    bool ok = false;

    cUnit &pUnit = unit[id];
    if (pUnit.getPlayer() == m_player) {
        if (pUnit.bSelected) {
            m_player->deselectUnit(id);

            // the deselected unit may have been last in its group
            if (pUnit.isHarvester()) {
                m_harvestersSelected = false;
            } else if (pUnit.isInfantryUnit()) {
                m_infantrySelected = false;
            } else {
                m_repairableUnitsSelected = false;
            }

            // does m_context->setMouseState on emptied selection
            changeSelectedUnits(m_player->getSelectedUnits());

            ok = true;
        }
    }

    return ok;
}

void cMouseUnitsSelectedState::changeSelectedUnits(const std::vector<int> &ids) {
    bool replace = m_state != SELECTED_STATE_ADD_TO_SELECTION;

    if (replace) {
        m_player->deselectAllUnits();
    }
    m_player->selectUnits(ids);
    updateSelectedUnitsState(ids, replace);

    if (!(m_harvestersSelected || m_infantrySelected || m_repairableUnitsSelected)) {
        // we get in a state where no units are selected,
        // so get back into "select" state.
        m_context->setMouseState(MOUSESTATE_SELECT);
    }
}

void cMouseUnitsSelectedState::onMouseLeftButtonClicked() {
    logbook(fmt::format(
        "cMouseUnitsSelectedState::onMouseLeftButtonClicked(): eval with state {}",
        mouseUnitsSelectedStateString(m_state)
    ));

    if (m_mouse->isBoxSelecting()) {
        cRectangle boxSelectRectangle = m_mouse->getBoxSelectRectangle();
        changeSelectedUnits(m_player->getAllMyUnitsWithinViewportRect(boxSelectRectangle));
    } else {
        // single click, no box select
        int mouseCell = m_context->getMouseCell();

        bool infantryAcknowledged = false;
        bool unitAcknowledged = false;

        if (m_state == SELECTED_STATE_SELECT) {
            // evaluateMouseMoveState set state on condition m_player is owner of selectable
            int hoverStructureId = m_context->getIdOfStructureWhereMouseHovers();
            if (hoverStructureId > -1) {
                m_player->selected_structure = hoverStructureId;
                cAbstractStructure *pStructure = m_player->getSelectedStructure();
                if (pStructure && pStructure->isValid()) {
                    m_player->getSideBar()->setSelectedListId(pStructure->getAssociatedListID());
                    changeSelectedUnits(std::vector<int>());
                } else {
                    m_player->selected_structure = -1;
                }
            } else {
                int hoverUnitId = m_context->getIdOfUnitWhereMouseHovers();
                if (hoverUnitId > -1) {
                    changeSelectedUnits(std::vector<int>(1, hoverUnitId));
                }
            }
        } else if (m_state == SELECTED_STATE_REPAIR ||
                    m_state == SELECTED_STATE_REFINERY ||
                    m_state == SELECTED_STATE_MOVE ||
                    m_state == SELECTED_STATE_CAPTURE) {

            for (const auto &id: m_player->getSelectedUnits()) {
                cUnit &pUnit = unit[id];
                if (m_state == SELECTED_STATE_REPAIR) {
                    // only send units that are eligible for repair to facility
                    if (pUnit.isEligibleForRepair()) {
                        pUnit.move_to(mouseCell);
                    }
                    unitAcknowledged = true;
                } else if (m_state == SELECTED_STATE_REFINERY) {
                    // only send harvesters in group
                    if (pUnit.isHarvester()) {
                        pUnit.move_to(mouseCell);
                    }
                    unitAcknowledged = true;
                } else {
                    if (pUnit.isInfantryUnit()) {
                        infantryAcknowledged = true;
                        pUnit.move_to(mouseCell);
                    } else if (m_state != SELECTED_STATE_CAPTURE) {
                        unitAcknowledged = true;
                        pUnit.move_to(mouseCell);
                    }
                }
            }
            spawnParticle(D2TM_PARTICLE_MOVE);
        } else if (m_state == SELECTED_STATE_ATTACK || m_state == SELECTED_STATE_FORCE_ATTACK) {
            for (const auto &id: m_player->getSelectedUnits()) {
                cUnit &pUnit = unit[id];
                if (!pUnit.isHarvester()) {
                    if (pUnit.isInfantryUnit()) {
                        infantryAcknowledged = true;
                    } else {
                        unitAcknowledged = true;
                    }
                    pUnit.attackAt(mouseCell);
                }
            }

            spawnParticle(D2TM_PARTICLE_ATTACK);
        } else if (m_state == SELECTED_STATE_ADD_TO_SELECTION) {
            const int hoverUnitId = m_context->getIdOfUnitWhereMouseHovers();
            if (hoverUnitId > -1) {
                if (!deselectUnit(hoverUnitId)) {
                    changeSelectedUnits(std::vector<int>(1, hoverUnitId));
                }
            }
        }

        if (infantryAcknowledged) {
            game.playSound(SOUND_MOVINGOUT + rnd(2));
        }

        if (unitAcknowledged) {
            game.playSound(SOUND_ACKNOWLEDGED + rnd(3));
        }
    }

    m_mouse->resetBoxSelect();
}

void cMouseUnitsSelectedState::onMouseRightButtonPressed() {
    m_mouse->dragViewportInteraction();
}

void cMouseUnitsSelectedState::onMouseRightButtonClicked() {
    // if we were dragging the viewport, keep the units and this state.
    if (!m_mouse->isMapScrolling()) {
        // back to "select" state
        changeSelectedUnits(std::vector<int>());
    }

    m_mouse->resetDragViewportInteraction();
}

void cMouseUnitsSelectedState::onMouseMiddleButtonClicked() {
    m_context->setMouseState(MOUSESTATE_REPAIR);
}

void cMouseUnitsSelectedState::onMouseMovedTo() {
    if (m_mouse->isBoxSelecting()) {
        mouseTile = MOUSE_NORMAL;
    } else if (m_state == SELECTED_STATE_FORCE_ATTACK) {
        mouseTile = MOUSE_ATTACK;
    } else if (m_state == SELECTED_STATE_ADD_TO_SELECTION) {
        mouseTile = MOUSE_NORMAL;

        int hoverUnitId = m_context->getIdOfUnitWhereMouseHovers();
        if (hoverUnitId > -1) {
            cUnit &pUnit = unit[hoverUnitId];
            if (pUnit.getPlayer() == m_player) {
                mouseTile = MOUSE_PICK;
            }
        }
    } else {
        evaluateMouseMoveState();
    }
}

void cMouseUnitsSelectedState::evaluateMouseMoveState() {
    mouseTile = MOUSE_MOVE;
    setState(SELECTED_STATE_MOVE);

    cAbstractStructure *hoverStructure = m_context->getStructurePointerWhereMouseHovers();
    if (hoverStructure) {
        if (!hoverStructure->getPlayer()->isSameTeamAs(m_player)) {
            if (m_infantrySelected && m_infantryShouldCapture) {
                setState(SELECTED_STATE_CAPTURE);
                mouseTile = MOUSE_LEFT; //MOUSE_MOVE;
            } else if (m_infantrySelected || m_repairableUnitsSelected) {
                mouseTile = MOUSE_ATTACK;
                setState(SELECTED_STATE_ATTACK);
            }
        } else if (hoverStructure->belongsTo(m_player)) {
            if (hoverStructure->getType() == REFINERY) {
                if (m_harvestersSelected) {
                    setState(SELECTED_STATE_REFINERY);
                } else {
                    setState(SELECTED_STATE_SELECT);
                    mouseTile = MOUSE_NORMAL; // allow "selecting" of structure, even though we have units selected
                }
            } else if (hoverStructure->getType() == REPAIR) {
                if (m_repairableUnitsSelected || m_harvestersSelected) {
                    setState(SELECTED_STATE_REPAIR);
                } else {
                    setState(SELECTED_STATE_SELECT);
                    mouseTile = MOUSE_NORMAL; // allow "selecting" of structure, even though we have units selected
                }
            } else {
                setState(SELECTED_STATE_SELECT);
                mouseTile = MOUSE_NORMAL; // allow "selecting" of structure, even though we have units selected
            }
        }
    }

    int hoverUnitId = m_context->getIdOfUnitWhereMouseHovers();
    if (hoverUnitId > -1) {
        cUnit &pUnit = unit[hoverUnitId];
        if (pUnit.isValid()) {
            if (!pUnit.getPlayer()->isSameTeamAs(m_player)) {
                // don't try to attack with harvesters
                if (m_infantrySelected || m_repairableUnitsSelected) {
                    mouseTile = MOUSE_ATTACK;
                    setState(SELECTED_STATE_ATTACK);
                }
            } else if (pUnit.getPlayer() == m_player) {
                mouseTile = MOUSE_PICK;
                setState(SELECTED_STATE_SELECT); // allow selecting of my unit
            }
        }
    }
}

void cMouseUnitsSelectedState::onStateSet() {
    m_infantryShouldCapture = false;
    updateSelectedUnitsState();
    evaluateMouseMoveState();
    m_mouse->setTile(mouseTile);
}

void cMouseUnitsSelectedState::updateSelectedUnitsState(const std::vector<int> &ids, bool reset) {
    if (reset) {
        m_harvestersSelected = false;
        m_infantrySelected = false;
        m_repairableUnitsSelected = false;
    }
    for (const auto &id: ids) {
        cUnit &pUnit = unit[id];
        if (pUnit.isHarvester()) {
            m_harvestersSelected = true;
        } else if (pUnit.isInfantryUnit()) {
            m_infantrySelected = true;
        } else {
            m_repairableUnitsSelected = true;
        }

        if (m_harvestersSelected && m_infantrySelected && m_repairableUnitsSelected) {
            // no need to evaluate further
            break;
        }
    }
}

void cMouseUnitsSelectedState::updateSelectedUnitsState() {
    updateSelectedUnitsState(m_player->getSelectedUnits(), true);
}

void cMouseUnitsSelectedState::setState(eMouseUnitsSelectedState newState) {
    if (newState != m_state) {
        logbook(fmt::format("cMouseUnitsSelectedState: Changed state from [{}] to [{}]", mouseUnitsSelectedStateString(m_state),
                mouseUnitsSelectedStateString(newState)));
        this->m_prevState = this->m_state;
        this->m_state = newState;
    }
}

void cMouseUnitsSelectedState::onNotifyKeyboardEvent(const cKeyboardEvent &event) {
    switch (event.eventType) {
        case eKeyEventType::HOLD:
            onKeyDown(event);
            break;
        case eKeyEventType::PRESSED:
            onKeyPressed(event);
            break;
        default:
            break;

    }

    if (m_state == SELECTED_STATE_MOVE) {

    }

    if (m_context->isState(MOUSESTATE_UNITS_SELECTED)) { // if , required in case we switched state
        m_mouse->setTile(mouseTile);
    }
}

void cMouseUnitsSelectedState::onKeyDown(const cKeyboardEvent &event) {
    if (event.hasKey(KEY_LCONTROL) || event.hasKey(KEY_RCONTROL)) {
        setState(SELECTED_STATE_FORCE_ATTACK);
        onMouseMovedTo();
    }

    bool appendingSelectionToGroup = event.hasKey(KEY_LSHIFT) || event.hasKey(KEY_RSHIFT);
    if (appendingSelectionToGroup) {
        setState(SELECTED_STATE_ADD_TO_SELECTION);
        onMouseMovedTo();

        int iGroup = event.getGroupNumber();

        // holding shift & group number, so add group to the selected units
        if (iGroup > 0) {
            changeSelectedUnits(m_player->getAllMyUnitsForGroupNr(iGroup));
        }
    } else {
        bool createGroup = event.hasKey(KEY_RCONTROL) || event.hasKey(KEY_LCONTROL);
        // Do this within the "HOLD" event, because if we do it at Pressed event
        // we miss the fact that we hold SHIFT as well (see cKeyboard for reason).
        if (!createGroup) {
            int iGroup = event.getGroupNumber();

            if (iGroup > 0) {
                // select all units for group
                changeSelectedUnits(m_player->getAllMyUnitsForGroupNr(iGroup));
            }
        }
    }

    if (event.hasKey(KEY_E) && !m_infantryShouldCapture) {
        m_infantryShouldCapture = true;
        onMouseMovedTo();
    }
    // force move?
}

void cMouseUnitsSelectedState::onKeyPressed(const cKeyboardEvent &event) {
    if (event.hasKey(KEY_LCONTROL) || event.hasKey(KEY_RCONTROL)) {
        // CTRL released while SHIFT still down
        if (m_prevState == SELECTED_STATE_FORCE_ATTACK) {
            m_prevState = SELECTED_STATE_MOVE;
        }
        if (m_state == SELECTED_STATE_FORCE_ATTACK) {
            toPreviousState();
            onMouseMovedTo();
        }
    }

    if (event.hasKey(KEY_LSHIFT) || event.hasKey(KEY_RSHIFT)) {
        // SHIFT released while CTRL still down
        if (m_prevState == SELECTED_STATE_ADD_TO_SELECTION) {
            m_prevState = SELECTED_STATE_MOVE;
        }
        if (m_state == SELECTED_STATE_ADD_TO_SELECTION) {
            toPreviousState();
            onMouseMovedTo();
        }
    }

    // leave capture intent
    if (event.hasKey(KEY_E)) {
        m_infantryShouldCapture = false;
        onMouseMovedTo();
    }

    // go to repair state
    if (event.hasKey(KEY_R)) {
        m_context->setMouseState(MOUSESTATE_REPAIR);
    }
    
    // order any selected harvester to return to refinery
    if (event.hasKey(KEY_D)) {
        for (const auto &id : m_player->getSelectedUnits()) {
            cUnit &pUnit = unit[id];
            if (pUnit.isHarvester() && pUnit.canUnload()) {
                pUnit.findBestStructureCandidateAndHeadTowardsItOrWait(REFINERY, true, INTENT_UNLOAD_SPICE);
            }
        }
    }

    // force move?
}

void cMouseUnitsSelectedState::toPreviousState() {
    m_state = m_prevState;
}

void cMouseUnitsSelectedState::spawnParticle(const int type) {
    int absoluteXCoordinate = mapCamera->getAbsMapMouseX(mouse_x);
    int absoluteYCoordinate = mapCamera->getAbsMapMouseY(mouse_y);
    cParticle::create(absoluteXCoordinate, absoluteYCoordinate, type, -1, -1);
}

void cMouseUnitsSelectedState::onFocus() {
    m_mouse->setTile(mouseTile);
}

void cMouseUnitsSelectedState::onBlur() {
    m_mouse->resetBoxSelect();
    m_mouse->resetDragViewportInteraction();
}
