#ifndef PulseCounter_h
#define PulseCounter_h
#include "Arduino.h"
class PulseCounter{
public:
    PulseCounter();
    float getValue();
    void pulseEnded();
    void setPulsesPerUnit(int pulsesPerUnit);
    int getPulsesPerUnit();
    int* getPulsesPerUnitPointer();
private:
    unsigned long _lastMeasurement;
    int _pulsesPerUnit;
    volatile unsigned int _countedPulses;
    float _value;
};
#endif
