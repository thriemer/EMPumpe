#include "PulseCounter.h"
#include "Arduino.h"

void PulseCounter::pulseEnded(){
    _countedPulses++;
}
float PulseCounter::getValue(){
    unsigned long time = millis();
    unsigned long delta = time-_lastMeasurement;
    _value = ((float(_countedPulses)/float(_pulsesPerUnit))/float(delta))*1000.0f;
    _lastMeasurement=time;
    _countedPulses=0;
 return _value;
}

void PulseCounter::setPulsesPerUnit(int pulsesPerUnit){
   _pulsesPerUnit=pulsesPerUnit;
}
int PulseCounter::getPulsesPerUnit(){
   return _pulsesPerUnit;   
}
int* PulseCounter::getPulsesPerUnitPointer(){
   return &_pulsesPerUnit;   
}

unsigned int PulseCounter::getCountedPulses(){
    return _countedPulses;
}

PulseCounter::PulseCounter(){
   _pulsesPerUnit=1;
   _value=0;
   _lastMeasurement=0;
}
