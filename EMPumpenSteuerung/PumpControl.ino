int minControlSignal = 10;

float getVelocity() {
  if (useSimulatedVelocity) {
    return (float)simulatedVelocity / 3.6f;
  } else {
    float val = traktorGeschwindigkeit.getValue();
    return val;
  }
}

float calculateWantedLitersPerSecondFromVelocity(float velocityInMeterPerSecond) {
  float literPerSquareMeter = literProHektar / 10000.0f;
  return literPerSquareMeter * velocityInMeterPerSecond * arbeitsBreiteInDezimeter/10.0f;
}

int calculatePumpControlSignalFromWantedLitersPerSecond(float neededFlowInLiterPerSecond) {
  float maxLiterPerSecond = maxLiterPerHour / ((float)60 * 60);
  float flowInZeroToOne = neededFlowInLiterPerSecond / maxLiterPerSecond;
  int controlSignal = (int)(flowInZeroToOne * 255);
  if (controlSignal < minControlSignal) {
    controlSignal = 0;
    //you could also signal drive faster here
  }
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

const float MAX_ALLOWED_DEVIATION = 0.1f;
int deviateMoreThanCount = 0;
bool isRepeatedDeviating(float actualFlow, float wantedFlow) {
  bool error = false;
  float deviation = 0;
  float sum = wantedFlow + actualFlow;
  if (sum > 0.001f) {
    deviation = abs(wantedFlow - actualFlow) / (sum / 2.0f);
  }
  if (deviation > MAX_ALLOWED_DEVIATION) {
    deviateMoreThanCount++;
  } else {
    deviateMoreThanCount = 0;
  }
  if (deviateMoreThanCount > 10) {
    error = true;
  }
  return error;
}

void resetErrorState() {
  deviateMoreThanCount = 0;
  digitalWrite(RED_LED_PIN, LOW);
}

bool isHeckKraftheberUnten() {
  //TODO: fill this function
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
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(WHITE_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
}

float berechneVerbrauch() {
  return summierterVerbrauch / (float)flussMesser.getPulsesPerUnit();
}

void driveMotor() {
  float velocity = getVelocity();
  float wantedFlow = calculateWantedLitersPerSecondFromVelocity(velocity);
  float actualFlow = readFlowSensor();
  bool flowSensorRepeatingError = isRepeatedDeviating(actualFlow, wantedFlow);
  int pumpControlSignal = calculatePumpControlSignalFromWantedLitersPerSecond(wantedFlow);
  if (!flowSensorRepeatingError && isHeckKraftheberUnten) {
    analogWrite(PUMP_PWM_PIN, pumpControlSignal);
    turnSystemReadyLedOn();
    if (pumpControlSignal > 0) {
      turnPumpLedOn();
    } else {
      turnPumpLedOff();
    }
  } else {
    analogWrite(PUMP_PWM_PIN, 0);
    turnRedLedOn();
  }
  lcd.setCursor(0, 0);
  lcd.print(useSimulatedVelocity ? "Simul. Geschw" : "Gemessene Geschw");
  lcd.print(velocity * 3.6f);
  lcd.setCursor(0, 1);
  lcd.print("Verbrauch: ");
  lcd.print(berechneVerbrauch());
  lcd.setCursor(0, 2);
  lcd.print("Gewollte  L/s: ");
  lcd.print(wantedFlow);
  lcd.setCursor(0, 3);
  lcd.print("Gemessene L/s: ");
  lcd.print(actualFlow);
}
