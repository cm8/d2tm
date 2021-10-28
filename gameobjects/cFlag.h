#ifndef D2TM_CFLAG_H
#define D2TM_CFLAG_H

#include <utils/cPoint.h>

class cPlayer;

class cFlag {

public:
    cFlag(cPlayer *player, cPoint &absCoords, int frames, int animationDelay);
    ~cFlag() = default;

    void draw();

    void thinkFast();

    static cFlag * createBigFlag(cPlayer * player, cPoint & position);

private:
    cPoint absCoords;

    cPlayer *player;
    int TIMER_animate;

    // move to 'type object' later (ie flag definitions)
    int animationDelay;
    int frameHeight;
    int frames; // max amount of frames

    int frame;  // frame to draw

};


#endif //D2TM_CFLAG_H
