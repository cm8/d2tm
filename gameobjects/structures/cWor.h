#pragma once

class cWor : public cAbstractStructure {
private:


public:
    cWor();

    ~cWor();

    // overloaded functions    
    void think() override;

    void think_animation() override;

    void think_guard() override;

    void startAnimating() override {};

    void draw() override { drawWithShadow(); }

    int getType() const override;

};

