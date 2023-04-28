#define READ_DELAY 2000

int _localChange = -1;
bool _exitedChangeIntVar = true;
bool _confirmedChangeIntVar = false;

int prevButtonDirection = 0;
int buttonHoldCount = 0;
const int changeVarLoopInterval = 100;
const int holdTimeUntilFast = 3000;
//divided by four because there are two additional unregistered loop
const int holdCountUntilFast = holdTimeUntilFast / changeVarLoopInterval / 4;
unsigned long prevButtonPressMillis = 0;

const char* VORGEWENDE_NAME[] = { "Deaktiviert ", "Ackerschiene", "Extern +    ", "Extern -    " };

void mFunc_setVorgewende(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    if (_exitedChangeIntVar) {
      _localChange = vorgewendeStatus;
      _exitedChangeIntVar = false;
      _confirmedChangeIntVar = false;
    }
    lcd.setCursor(0, 0);
    lcd.print(F("Vorgewende"));  // print some content on first row
    LCDML.FUNC_setLoopInterval(changeVarLoopInterval);
  }
  int minVal = 0;
  int maxVal = 3;

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    if (LCDML.BT_checkDown()) {
      LCDML.BT_resetDown();
      _localChange--;
      if (_localChange < minVal) {
        _localChange = maxVal;
      }
    }
    if (LCDML.BT_checkUp()) {
      LCDML.BT_resetUp();
      _localChange++;
      if (_localChange > maxVal) {
        _localChange = minVal;
      }
    }
    if (LCDML.BT_checkEnter()) {
      _confirmedChangeIntVar = true;
      LCDML.FUNC_goBackToMenu();
    }
    lcd.setCursor(0, 1);
    lcd.print(VORGEWENDE_NAME[_localChange]);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd.clear();
    if (vorgewendeStatus != _localChange && _confirmedChangeIntVar) {
      vorgewendeStatus = _localChange;
      lcd.setCursor(4, 0);
      lcd.print(F("GESPEICHERT!"));
      lcd.setCursor(5, 1);
      lcd.print(F("Neuer Wert:"));
      lcd.setCursor(4, 2);
      lcd.print(VORGEWENDE_NAME[vorgewendeStatus]);
      writeIntIntoEEPROM(EPROM_VORGEWENDE_STATUS, vorgewendeStatus);
    } else {
      lcd.setCursor(7, 1);
      lcd.print(F("KEINE"));
      lcd.setCursor(5, 2);
      lcd.print(F("AENDERUNG"));
    }
    _exitedChangeIntVar = true;
    _confirmedChangeIntVar = false;
    buttonHoldCount = 0;
    interuptableWait();
  }
}

void changeIntVar(char* varName, int* toChange, int minVal, int maxVal, float multiplier, int address, uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    if (_exitedChangeIntVar) {
      _localChange = *toChange;
      _exitedChangeIntVar = false;
      _confirmedChangeIntVar = false;
    }
    lcd.setCursor(0, 0);
    lcd.print(varName);  // print some content on first row
    LCDML.FUNC_setLoopInterval(changeVarLoopInterval);
  }

  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    int change = buttonHoldCount < holdCountUntilFast ? 1 : 10;
    int button = 0;
    if (LCDML.BT_checkDown()) {
      button = -1;
      LCDML.BT_resetDown();
      _localChange -= change;
      if (_localChange < minVal) {
        _localChange = maxVal;
      }
    }
    if (LCDML.BT_checkUp()) {
      button = 1;
      LCDML.BT_resetUp();
      _localChange += change;
      if (_localChange > maxVal) {
        _localChange = minVal;
      }
    }
    if (LCDML.BT_checkEnter()) {
      button = 0;
      _confirmedChangeIntVar = true;
      LCDML.FUNC_goBackToMenu();
    }
    holdButtonLogic(button);
    lcd.setCursor(0, 1);
    lcd.print(_localChange * multiplier);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd.clear();
    if (*toChange != _localChange && _confirmedChangeIntVar) {
      *toChange = _localChange;
      lcd.setCursor(4, 0);
      lcd.print(F("GESPEICHERT!"));
      lcd.setCursor(5, 1);
      lcd.print(F("Neuer Wert:"));
      lcd.setCursor(8, 2);
      lcd.print(*toChange * multiplier);
      writeIntIntoEEPROM(address, *toChange);
    } else {
      lcd.setCursor(7, 1);
      lcd.print(F("KEINE"));
      lcd.setCursor(5, 2);
      lcd.print(F("AENDERUNG"));
    }
    _exitedChangeIntVar = true;
    _confirmedChangeIntVar = false;
    buttonHoldCount = 0;
    interuptableWait();
  }
}

void interuptableWait() {
  unsigned long waitStart = millis();
  delay(100);
  while (millis() - waitStart < READ_DELAY && digitalRead(_LCDML_CONTROL_digital_quit) == HIGH && digitalRead(_LCDML_CONTROL_digital_enter) == HIGH) {
    delay(1);
  }
  delay(100);
}

void holdButtonLogic(int button) {
  if (button != 0) {
    if (button == prevButtonDirection && millis() - prevButtonPressMillis < 4 * changeVarLoopInterval) {
      buttonHoldCount++;
    } else {
      buttonHoldCount = 0;
    }
    if (button != 0) {
      prevButtonPressMillis = millis();
    }
    prevButtonDirection = button;
  }
}

void mFunc_setArbeitsbreite(uint8_t param) {
  changeIntVar("Arbeitsbreite", &arbeitsBreiteInDezimeter, 0, 240, 0.1f, EPROM_ARBEITSBREITE, param);
}

void mFunc_setLiterProHektar(uint8_t param) {
  changeIntVar("Liter/Hektar", &literProHektar, 0, 1000, 1, EPROM_LITER_PRO_HEKTAR, param);
}

void mFunc_setPulsesPerLiter(uint8_t param) {
  changeIntVar("Pulses/Liter", flussMesser.getPulsesPerUnitPointer(), 0, 1000, 1, EPROM_FLUSSMESSER_PULSE_PER_LITER, param);
}

void mFunc_setPulsesPerMeter(uint8_t param) {
  changeIntVar("Pulses/Meter", traktorGeschwindigkeit.getPulsesPerUnitPointer(), 0, 1000, 1, EPROM_TRAKTOR_PULSE_PER_METER, param);
}
void mFunc_setSimulatedSpeed(uint8_t param) {
  changeIntVar("Simul. Geschw.", &simulatedVelocity, 1, 55, 1, EPROM_SIMULATED_SPEED, param);
}
void mFunc_setMaxLiterPerHour(uint8_t param) {
  changeIntVar("Pumpe max L/h", &maxLiterPerHour, 1, 2000, 1, EPROM_MAX_LITER_PER_HOUR, param);
  setPIDValues();
}

void mFunc_setPIDProportional(uint8_t param) {
  changeIntVar("PID Proportional", &pidProportional, 0, 100, 0.1f, EPROM_PID_PROPORTIONAL, param);
  setPIDValues();
}

void mFunc_setPIDIntegral(uint8_t param) {
  changeIntVar("PID Integral", &pidIntegral, 0, 100, 0.1f, EPROM_PID_INTEGRAL, param);
  setPIDValues();
}

void mFunc_setPIDDerivative(uint8_t param) {
  changeIntVar("PID Ableitung", &pidDerivative, 0, 100, 0.1f, EPROM_PID_DERIVATIVE, param);
  setPIDValues();
}

int _savedTraktorPulse = -1;
bool _exitedChangePulsePerMeterVelocity = true;
bool _confirmedChangePulsePerMeterVelocity = false;
void mFunc_setPulsesPerMeterVelocity(uint8_t param) {
  int minVal = 1;
  int maxVal = 1000;
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    if (_exitedChangePulsePerMeterVelocity) {
      _savedTraktorPulse = *traktorGeschwindigkeit.getPulsesPerUnitPointer();
      _exitedChangePulsePerMeterVelocity = false;
      _confirmedChangePulsePerMeterVelocity = false;
    }
    lcd.setCursor(0, 0);
    lcd.print("Geschwindigkeit");  // print some content on first row
    LCDML.FUNC_setLoopInterval(changeVarLoopInterval);
  }


  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    int change = buttonHoldCount < holdCountUntilFast ? 1 : 10;
    int button = 0;
    if (LCDML.BT_checkUp()) {
      LCDML.BT_resetUp();
      button = -1;
      *traktorGeschwindigkeit.getPulsesPerUnitPointer() -= change;
      if (*traktorGeschwindigkeit.getPulsesPerUnitPointer() < minVal) {
        *traktorGeschwindigkeit.getPulsesPerUnitPointer() = minVal;
      }
    }
    if (LCDML.BT_checkDown()) {
      LCDML.BT_resetDown();
      button = 1;
      *traktorGeschwindigkeit.getPulsesPerUnitPointer() += change;
      if (*traktorGeschwindigkeit.getPulsesPerUnitPointer() > maxVal) {
        *traktorGeschwindigkeit.getPulsesPerUnitPointer() = maxVal;
      }
    }
    if (LCDML.BT_checkEnter()) {
      button = 0;
      _confirmedChangePulsePerMeterVelocity = true;
      LCDML.FUNC_goBackToMenu();
    }
    holdButtonLogic(button);
    lcd.setCursor(0, 1);
    lcd.print(F("Pulse: "));
    lcd.print(*traktorGeschwindigkeit.getPulsesPerUnitPointer());
    lcd.setCursor(8, 1);
    lcd.setCursor(0, 2);
    lcd.print(F("Geschw: "));
    lcd.print(traktorGeschwindigkeit.getValue() * 3.6f);
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    lcd.clear();
    if (*traktorGeschwindigkeit.getPulsesPerUnitPointer() != _savedTraktorPulse && _confirmedChangePulsePerMeterVelocity) {
      lcd.setCursor(4, 0);
      lcd.print("GESPEICHERT!");
      lcd.setCursor(5, 1);
      lcd.print("Neuer Wert:");
      lcd.setCursor(8, 2);
      lcd.print(traktorGeschwindigkeit.getValue() * 3.6f);
      writeIntIntoEEPROM(EPROM_TRAKTOR_PULSE_PER_METER, *traktorGeschwindigkeit.getPulsesPerUnitPointer());
    } else {
      *traktorGeschwindigkeit.getPulsesPerUnitPointer() = _savedTraktorPulse;
      lcd.setCursor(7, 1);
      lcd.print("KEINE");
      lcd.setCursor(5, 2);
      lcd.print("AENDERUNG");
    }
    _exitedChangePulsePerMeterVelocity = true;
    _confirmedChangePulsePerMeterVelocity = false;
    buttonHoldCount = 0;
    interuptableWait();
  }
}

void mFunc_Verbrauch(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    lcd.setCursor(1, 1);
    lcd.print(F("Verbrauch in Liter"));
    lcd.setCursor(6, 2);
    float liter = berechneVerbrauch();
    lcd.print(liter);
  }
  if (LCDML.FUNC_loop()) {
    if (LCDML.BT_checkAny())  // check if any button is pressed (enter, up, down, left, right)
    {
      LCDML.FUNC_goBackToMenu();  // leave this function
    }
  }
}

long startedMillis = 0;
void mFunc_VerbrauchZuruecksetzen(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    startedMillis = millis();
    LCDML_UNUSED(param);
    summierterVerbrauch = 0;
    writeUnsignedLongIntoEEPROM(EPROM_VERBRAUCH_GESAMMT, summierterVerbrauch);
    lcd.setCursor(5, 1);
    lcd.print("Verbrauch");
    lcd.setCursor(3, 2);
    lcd.print("zurueckgesetzt");
    interuptableWait();
    LCDML.FUNC_goBackToMenu();
  }
}

void driverMenu(uint8_t param) {
  if (LCDML.FUNC_setup())  // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    resetErrorState();
    lcd.clear();
    LCDML.FUNC_setLoopInterval(1000);
  }
  if (LCDML.FUNC_loop())  // ****** LOOP *********
  {
    driveMotor();
  }

  if (LCDML.FUNC_close())  // ****** STABLE END *********
  {
    resetErrorState();
    analogWrite(PUMP_PWM_PIN, 0);
    turnPumpLedOff();
    turnSystemReadyLedOff();
    resetPIDController();
  }
}

void resetPIDController() {
  pumpSetPoint = 0;
  flowSensorReading = 0;
  pumpControl = 0;
  pumpPID.reset();
}

void mFunc_startWithRealSpeed(uint8_t param) {
  useSimulatedVelocity = false;
  driverMenu(param);
}

void mFunc_startWithSimulatedSpeed(uint8_t param) {
  useSimulatedVelocity = true;
  driverMenu(param);
}
