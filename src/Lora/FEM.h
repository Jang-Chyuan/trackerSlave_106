#pragma once

#include <Arduino.h>

class FEMControl
{

public:
    void begin();

    void txMode();

    void rxMode();

    void setRxLnaEnabled(bool enabled);

    bool isRxLnaEnabled() const;

    void standby();

private:
    bool rxLnaEnabled = true;
    void sleep(); // 加這行
};

extern FEMControl FEM;
