int minControlSignal = 10;
#define HEKTAR_AREA 10000.0d
float getVelocity() {
  if (useSimulatedVelocity) {
    return (float)simulatedVelocity / 3.6f;
  } else {
    float val = traktorGeschwindigkeit.getValue();
    return val;
  }
}

double calculateWantedLitersPerSecondFromVelocity(float velocityInMeterPerSecond) {
  double literPerSquareMeter = literProHektar / HEKTAR_AREA;
  return literPerSquareMeter * velocityInMeterPerSecond * arbeitsBreiteInDezimeter / 10.0d;
}

int calculatePumpControlSignalFromWantedLitersPerSecond(float neededFlowInLiterPerSecond) {
  float maxLiterPerSecond = maxLiterPerHour / ((float)60 * 60);
  float flowInZeroToOne = neededFlowInLiterPerSecond / maxLiterPerSecond;
  int controlSignal = (int)(flowInZeroToOne * 255);
  if (controlSignal < minControlSignal) {
    controlSignal = 0;
    //you could also signal drive faster here
  }
  moreFlowThanPossible = controlSignal > 255;
  if (controlSignal > 255) {
    controlSignal = 255;
    //something is possibly go wrong. Turn lamp on?
  }
  return controlSignal;
}
//den Verbrauch nur aller zwei Minuten speichern, weil der EEPROM nur 100.000 geschrieben werden kann
//angenommen der Arduino wird 60 Tage im Jahr, acht Stunden am Tag benutzt. Wenn man aller zwei Minuten speichert, dann hält diese Adresse ~7 Jahre
long nextmillisSavedVerbrauch = 0;
void addVerbrauch() {
  long now = millis();
  summierterVerbrauch += flussMesser.getCountedPulses();
  if (now >= nextmillisSavedVerbrauch) {
    writeUnsignedLongIntoEEPROM(EPROM_VERBRAUCH_GESAMMT, summierterVerbrauch);
    nextmillisSavedVerbrauch = now + 2L * 60L * 1000L;
  }
}

float readFlowSensor() {
  addVerbrauch();
  float actualLitersPerSecond = flussMesser.getValue();
  return actualLitersPerSecond;
}

const float MAX_ALLOWED_DEVIATION = 0.2f;
int deviateMoreThanCount = 0;
bool isRepeatedDeviating(float actualFlow, float wantedFlow) {
  float deviation = 0;
  if (wantedFlow > 0.01f) {
    deviation = abs(wantedFlow - actualFlow) / wantedFlow;
  }
  if (deviation > MAX_ALLOWED_DEVIATION) {
    deviateMoreThanCount++;
  } else {
    deviateMoreThanCount = 0;
  }
  return deviateMoreThanCount >= 5;
}

void resetErrorState() {
  deviateMoreThanCount = 0;
  repeatedDeviating = false;
  digitalWrite(RED_LED_PIN, LOW);
}

bool isHeckKraftheberUnten() {
  //manual control, dont intercept
  if (vorgewendeStatus == DISABLED) return true;
  if (vorgewendeStatus == ACKERSCHIENE) {
    return digitalRead(ACKERSCHIENE_INPUT) == LOW;
  }
  if(vorgewendeStatus == EXTERN){
    return digitalRead(VORGEWENDE_EXTERN_INPUT) == LOW;
  }
  if(vorgewendeStatus == EXTERN_INVERTED){
    return digitalRead(VORGEWENDE_EXTERN_INPUT) == HIGH;
  }
  return true;
}

void turnPumpLedOn() {
  digitalWrite(GREEN_LED_PIN, HIGH);
}
void turnPumpLedOff() {
  digitalWrite(GREEN_LED_PIN, LOW);
}

void turnSystemReadyLedOn() {
  digitalWrite(WHITE_LED_PIN, HIGH);
}
void turnSystemReadyLedOff() {
  digitalWrite(WHITE_LED_PIN, LOW);
}

void turnRedLedOn() {
  digitalWrite(RED_LED_PIN, HIGH);
}
void turnRedLedOff() {
  digitalWrite(RED_LED_PIN, LOW);
}


float berechneVerbrauch() {
  return summierterVerbrauch / (float)flussMesser.getPulsesPerUnit();
}

char verbrauchBuffer[6];
void driveMotor() {
  float velocity = getVelocity();
  flowSensorReading = readFlowSensor();
  bool heckKraftheberUntenStatus = isHeckKraftheberUnten();
  if (heckKraftheberUntenStatus) {
    pumpSetPoint = calculateWantedLitersPerSecondFromVelocity(velocity);
    pumpPID.Compute();
    float wantedFlow = (float)pumpControl;
    int pumpControlSignal = calculatePumpControlSignalFromWantedLitersPerSecond(wantedFlow);
    analogWrite(PUMP_PWM_PIN, pumpControlSignal);
    turnSystemReadyLedOn();
    if (pumpControlSignal > 0) {
      turnPumpLedOn();
    } else {
      turnPumpLedOff();
    }
  } else {
    pumpSetPoint = 0;
    analogWrite(PUMP_PWM_PIN, 0);
    turnPumpLedOff();
  }
  repeatedDeviating = isRepeatedDeviating(flowSensorReading, (float)pumpSetPoint);
  lcd.setCursor(0, 0);
  lcd.print(velocity * 3.6f);
  lcd.print(F(" Km/h"));
  lcd.setCursor(12, 0);
  snprintf(verbrauchBuffer, sizeof verbrauchBuffer, "%04d", (int)berechneVerbrauch());
  lcd.print(verbrauchBuffer);
  lcd.print(F(" L"));
  lcd.setCursor(0, 1);
  lcd.print(F("Soll: "));
  lcd.print(pumpSetPoint * 60);
  lcd.print(F(" L/min"));
  lcd.setCursor(0, 2);
  lcd.print(F("Ist : "));
  lcd.print(flowSensorReading * 60);
  lcd.print(F(" L/min"));
  lcd.setCursor(0, 3);
  lcd.print(F("Ist : "));
  double timeForOneHektar = velocity > 0.01 ? HEKTAR_AREA / (arbeitsBreiteInDezimeter / 10.0d * velocity) : 1000;
  lcd.print(flowSensorReading * timeForOneHektar);
  lcd.print(F(" L/ha"));

  lcd.setCursor(19, 3);
  if(vorgewendeStatus == DISABLED){
    lcd.print("X");
  }else{
    lcd.print((heckKraftheberUntenStatus ? (char)6 : (char)5));
  }
}
