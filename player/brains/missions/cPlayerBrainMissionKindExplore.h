#ifndef D2TM_CPLAYERBRAINMISSIONKIND_EXPLORE_H
#define D2TM_CPLAYERBRAINMISSIONKIND_EXPLORE_H

#include "player/playerh.h"
#include "cPlayerBrainMissionKind.h"

namespace brains {

    class cPlayerBrainMission;

    class cPlayerBrainMissionKindExplore : public cPlayerBrainMissionKind {

    public:

        cPlayerBrainMissionKindExplore(cPlayer *player, cPlayerBrainMission * mission);

        ~cPlayerBrainMissionKindExplore() override;

        cPlayerBrainMissionKind * clone(cPlayer *player, cPlayerBrainMission * mission) override;

        bool think_SelectTarget() override;

        void think_Execute() override;

        void onNotify(const s_GameEvent &event) override;

        const char *toString() override { return "cPlayerBrainMissionKindExplore"; }

    private:
        int targetCell;
    };

}

#endif //D2TM_CPLAYERBRAINMISSIONKIND_EXPLORE_H
