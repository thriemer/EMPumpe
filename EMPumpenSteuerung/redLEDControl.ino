unsigned long previousMillis;
int interval = 200;
int redLedState = LOW;

void blinkRedLED(){
    unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (redLedState == LOW) {
      redLedState = HIGH;
    } else {
      redLedState = LOW;
    }
  digitalWrite(RED_LED_PIN, redLedState);
  }
}

void controlRedLED(){
  if(repeatedDeviating){
    blinkRedLED();
  }else if(moreFlowThanPossible){
    turnRedLedOn();
  }else{
    turnRedLedOff();
  }
}