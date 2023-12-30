#pragma once

#include <utils/cPoint.h>

class cPlayer;

class cFlag {

public:
    cFlag(cPlayer *player, cPoint &absCoords, int frames, int animationDelay);
    ~cFlag() = default;

    void draw();

    void thinkFast();

    static cFlag * createBigFlag(cPlayer * player, cPoint & position);
    static cFlag * createSmallFlag(cPlayer * player, cPoint & position);

    void setBig(bool value) { big = value; }
    void setPlayer(cPlayer * player) { this->player = player; }
private:
    cPoint absCoords;

    cPlayer *player;
    int TIMER_animate;

    bool big;

    // move to 'type object' later (ie flag definitions)
    int animationDelay;
    int frames; // max amount of frames

    int frame;  // frame to draw

};
