#ifndef PulseCounter_h
#define PulseCounter_h
#include "Arduino.h"
class PulseCounter{
public:
    PulseCounter();
    float getValue();
    void pulseEnded();
    void setPulsesPerUnit(unsigned int pulsesPerUnit);
    unsigned int getPulsesPerUnit();
private:
    unsigned long _lastMeasurement;
    unsigned int _pulsesPerUnit;
    volatile unsigned int _countedPulses;
    float _value;
};
#endif
