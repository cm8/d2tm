#ifndef D2TM_CMOUSEUNITSSELECTEDSTATE_H
#define D2TM_CMOUSEUNITSSELECTEDSTATE_H

#include "cMouseState.h"

enum eMouseUnitsSelectedState {
    SELECTED_STATE_MOVE, // move selected units to location
    SELECTED_STATE_ATTACK, // attack target with selected units
    SELECTED_STATE_REPAIR, // enter structure to repair units
    SELECTED_STATE_CAPTURE, // enter structure to capture it
    SELECTED_STATE_REFINERY, // enter refinery to dump spice / collect credits
};

static const char* mouseUnitsSelectedStateString(const eMouseUnitsSelectedState &state) {
    switch (state) {
        case SELECTED_STATE_MOVE: return "SELECTED_STATE_MOVE";
        case SELECTED_STATE_ATTACK: return "SELECTED_STATE_ATTACK";
        case SELECTED_STATE_REPAIR: return "SELECTED_STATE_REPAIR";
        case SELECTED_STATE_CAPTURE: return "SELECTED_STATE_CAPTURE";
        case SELECTED_STATE_REFINERY: return "SELECTED_STATE_REFINERY";
        default:
            assert(false);
            break;
    }
    return "";
}

/**
 * A mouse "units selected" state is at the battlefield, and it is active when units have been selected (box, or single).
 * This state basically determines what kind of actions can be performed by the selected units. Ie, attacking an enemy
 * structure/unit. Or send (a part of) the selected units to dump spice in refinery, or get repaired.
 *
 * This state object has its own "state" to tell what kind of action is applicable, even though the rendered
 * mouseTile might be the same. Ie, there is (for now) no different mouseTile rendered when a unit can enter a
 * structure or when it is a 'normal move order'.
 *
 */
class cMouseUnitsSelectedState : public cMouseState {

public:
    explicit cMouseUnitsSelectedState(cPlayer * player, cGameControlsContext *context, cMouse * mouse);
    ~cMouseUnitsSelectedState() override;

    void onNotifyMouseEvent(const s_MouseEvent &event) override;
    void onNotifyKeyboardEvent(const s_KeyboardEvent &event) override;
    void onStateSet() override;

private:
    void onMouseLeftButtonClicked(const s_MouseEvent &event);

    void onMouseRightButtonPressed(const s_MouseEvent &event);

    void onMouseRightButtonClicked(const s_MouseEvent &event);

    void onMouseMovedTo(const s_MouseEvent &event);

    // A list of unit id's that we have selected
    std::vector<int> selectedUnits;

    // when move to refinery, it will only be applicable to harvesters.
    bool harvestersSelected;

    // todo: when we want to capture structures, it will only send out the infantry
    bool infantrySelected;

    // when sending to repair facility, only repairable units go to that
    bool repairableUnitsSelected;

    void updateSelectedUnitsState();

    void evaluateSelectedUnits();

    void setState(eMouseUnitsSelectedState newState);

    eMouseUnitsSelectedState state;
};


#endif //D2TM_CMOUSEUNITSSELECTEDSTATE_H
