#include "include/d2tmh.h"

#include "cPlayerBrainMissionKind.h"
#include "cPlayerBrainMissionKindAttack.h"

namespace brains {

    cPlayerBrainMissionKindAttack::cPlayerBrainMissionKindAttack(cPlayer *player, cPlayerBrainMission * mission) :  cPlayerBrainMissionKind(player, mission) {
        targetStructureID = -1;
        targetUnitID = -1;
        logbook("cPlayerBrainMissionKindAttack() constructor");
    }

    cPlayerBrainMissionKindAttack::~cPlayerBrainMissionKindAttack() {

    }

    void cPlayerBrainMissionKindAttack::think_SelectTarget() {
        // and execute whatever?? (can merge with select target state?)
        for (int i = 0; i < MAX_STRUCTURES; i++) {
            cAbstractStructure *theStructure = structure[i];
            if (!theStructure) continue;
            if (!theStructure->isValid()) continue;
            if (theStructure->getPlayer()->isSameTeamAs(player)) continue; // skip allies and self
            // enemy structure
            targetStructureID = theStructure->getStructureId();
            if (rnd(100) < 25) {
                break; // this way we kind of have randomly another target...
            }
        }
    }

    void cPlayerBrainMissionKindAttack::think_Execute() {
        if (targetStructureID < 0 && targetUnitID < 0) {
            // this should not happen!
            assert(false);
        }

        if (targetStructureID > -1) {
            cAbstractStructure *pStructure = structure[targetStructureID];
            assert(pStructure != nullptr);
            const std::vector<int> &units = mission->getUnits();
            for (auto &myUnit : units) {
                cUnit &aUnit = unit[myUnit];
                if (aUnit.isValid() && aUnit.isIdle()) {
                    logbook("cPlayerBrainMission::thinkState_Execute(): Ordering unit to attack!");
                    UNIT_ORDER_ATTACK(myUnit, pStructure->getCell(), -1, targetStructureID, -1);
                }
            }
        } else if (targetUnitID > -1) {
            cUnit &targetUnit = unit[targetUnitID];
            const std::vector<int> &units = mission->getUnits();
            for (auto &myUnit : units) {
                cUnit &aUnit = unit[myUnit];
                if (aUnit.isValid() && aUnit.isIdle()) {
                    logbook("cPlayerBrainMission::thinkState_Execute(): Ordering unit to attack!");
                    UNIT_ORDER_ATTACK(myUnit, targetUnit.getCell(), targetUnitID, -1, -1);
                }
            }
        }
    }

    void cPlayerBrainMissionKindAttack::onNotify(const s_GameEvent &event) {
        char msg[255];
        sprintf(msg, "cPlayerBrainMissionKindAttack::onNotify(), for player [%d] -> %s", player->getId(), event.toString(event.eventType));
        logbook(msg);

        switch(event.eventType) {
            case GAME_EVENT_DESTROYED:
                onEventDestroyed(event);
                break;
            case GAME_EVENT_DEVIATED:
                onEventDeviated(event);
                break;
        }
    }

    void cPlayerBrainMissionKindAttack::onEventDeviated(const s_GameEvent &event) {
        if (event.entityType == UNIT) {
            cUnit &entityUnit = unit[event.entityID];
            if (entityUnit.getPlayer() == player) {
                // the unit is ours, if it was a target, then we can forget it.
                if (targetUnitID == event.entityID) {
                    // our target got deviated, so it is no longer a threat
                    targetUnitID = -1;
                    mission->changeState(PLAYERBRAINMISSION_STATE_SELECT_TARGET);
                }
            }
        }
    }

    void cPlayerBrainMissionKindAttack::onEventDestroyed(const s_GameEvent &event) {
        if (event.entityType == UNIT) {
            if (event.player != player) {
                // our target got destroyed?
                if (targetUnitID == event.entityID) {
                    targetUnitID = -1;
                    mission->changeState(PLAYERBRAINMISSION_STATE_SELECT_TARGET);
                }
            }
        } else if (event.entityType == STRUCTURE) {
            if (event.player != player) {
                // our target got destroyed
                if (targetStructureID == event.entityID) {
                    targetStructureID = -1;
                    mission->changeState(PLAYERBRAINMISSION_STATE_SELECT_TARGET);
                }
            }
        }
    }

    cPlayerBrainMissionKind *cPlayerBrainMissionKindAttack::clone(cPlayer *player, cPlayerBrainMission * mission) {
        cPlayerBrainMissionKindAttack *copy = new cPlayerBrainMissionKindAttack(player, mission);
        copy->targetUnitID = targetUnitID;
        copy->targetStructureID = targetStructureID;
        return copy;
    }

}