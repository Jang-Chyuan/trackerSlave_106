#include "FEM.h"
#include "AppConfig.h"
#include "BoardConfig.h"
#include <Arduino.h>
// #include "driver/rtc_io.h"

FEMControl FEM;

void FEMControl::begin()
{

#ifdef USE_KCT8103L_PA

    // release RTC hold after deep sleep
    // rtc_gpio_hold_dis((gpio_num_t)LORA_PA_POWER);
    // rtc_gpio_hold_dis((gpio_num_t)LORA_PA_CSD);

    pinMode(LORA_PA_POWER, OUTPUT);
    pinMode(LORA_PA_CSD, OUTPUT);
    pinMode(LORA_PA_CTX, OUTPUT);

    // Enable FEM power
    digitalWrite(LORA_PA_POWER, HIGH);

    // Enable FEM
    digitalWrite(LORA_PA_CSD, HIGH);

    setRxLnaEnabled(LORA_FEM_LNA_ENABLED != 0);

    // default RX mode
    rxMode();

#endif
}

void FEMControl::txMode()
{

#ifdef USE_KCT8103L_PA

    // PA TX mode
    digitalWrite(LORA_PA_CTX, HIGH);
    delay(2);
#endif
}

void FEMControl::rxMode()
{

#ifdef USE_KCT8103L_PA
    // Tracker V2 KCT8103L: LOW = RX LNA, HIGH = RX bypass.
    digitalWrite(
        LORA_PA_CTX,
        rxLnaEnabled ? LOW : HIGH);
    delay(1);
#endif
}

void FEMControl::setRxLnaEnabled(bool enabled)
{
    rxLnaEnabled = enabled;
}

bool FEMControl::isRxLnaEnabled() const
{
    return rxLnaEnabled;
}

void FEMControl::standby()
{

#ifdef USE_KCT8103L_PA
    // Return to the configured receive path in standby.
    digitalWrite(
        LORA_PA_CTX,
        rxLnaEnabled ? LOW : HIGH);
#endif
}

void FEMControl::sleep()
{

#ifdef USE_KCT8103L_PA
    digitalWrite(LORA_PA_CTX, LOW);
    digitalWrite(LORA_PA_CSD, LOW);
    digitalWrite(LORA_PA_POWER, LOW);
#endif
}
