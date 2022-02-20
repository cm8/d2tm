#pragma once

#include "cAbstractStructure.h"

class cHeavyFactory : public cAbstractStructure {
private:
    void think_animation_unitDeploy();

    bool drawFlash;
    int flashes;

    int TIMER_animation;

public:
    cHeavyFactory();

    // overloaded functions    
    void thinkFast() override;

    void thinkSlow() override {}

    void think_animation() override;

    void think_guard() override;

    void startAnimating() override;

    void draw() override;

    int getType() const override;

    std::string getStatusForMessageBar() const override { return getDefaultStatusMessageBar(); }
};

