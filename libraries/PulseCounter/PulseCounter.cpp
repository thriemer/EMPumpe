#include "PulseCounter.h"
#include "Arduino.h"

void PulseCounter::pulseEnded(){
    _countedPulses++;
}
float PulseCounter::getValue(){
    unsigned long time = millis();
    unsigned long delta = time-_lastMeasurement;
    if(delta>1000){
        _value = float(_countedPulses)/float(_pulsesPerUnit)/float(delta)*1000.0f;
        _countedPulses=0;
        _lastMeasurement=time;
    }

 return _value;
}

void PulseCounter::setPulsesPerUnit(unsigned int pulsesPerUnit){
   _pulsesPerUnit=pulsesPerUnit;
}
unsigned int PulseCounter::getPulsesPerUnit(){
   return _pulsesPerUnit;   
}

PulseCounter::PulseCounter(){
   _pulsesPerUnit=1;
   _value=0;
   _lastMeasurement=0;
}
