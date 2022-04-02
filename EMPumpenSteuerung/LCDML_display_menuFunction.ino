#define READ_DELAY 2000
// *********************************************************************
void mFunc_showParameter(uint8_t param)
// *********************************************************************
{
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // setup function
    lcd.setCursor(0, 0);
    lcd.print(F("Arbeitsbreite:"));
    lcd.setCursor(15, 0);
    lcd.print(arbeitsBreiteInDezimeter / 10.0f);
    lcd.setCursor(0, 1);
    lcd.print(F("L/Ha:"));
    lcd.setCursor(6, 1);
    lcd.print(literProHektar);
  }

  if (LCDML.FUNC_loop())          // ****** LOOP *********
  {
    // loop function, can be run in a loop when LCDML_DISP_triggerMenu(xx) is set
    // the quit button works in every DISP function without any checks; it starts the loop_end function
    if (LCDML.BT_checkAny()) { // check if any button is pressed (enter, up, down, left, right)
      // LCDML_goToMenu stops a running menu function and goes to the menu
      LCDML.FUNC_goBackToMenu();
    }
  }

  if (LCDML.FUNC_close())     // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}

String appendSpace(String s) {
  int toAppend = 20 - s.length();
  for (int i = 0; i < toAppend; i++) {
    s += " ";
  }
  return s;
}

int _localChange = -1;
bool _exitedChangeIntVar = true;
bool _confirmedChangeIntVar = false;
void changeIntVar(String varName, int* toChange, int minVal, int maxVal, int change, float multiplier, int address, uint8_t param) {
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    if (_exitedChangeIntVar) {
      _localChange = *toChange;
      _exitedChangeIntVar = false;
      _confirmedChangeIntVar = false;
    }
    lcd.setCursor(0, 0);
    lcd.print(varName); // print some content on first row
    LCDML.FUNC_setLoopInterval(100);
  }


  if (LCDML.FUNC_loop())          // ****** LOOP *********
  {
    if (LCDML.BT_checkDown()) {
      LCDML.BT_resetDown();
      _localChange -= change;
      if (_localChange < minVal) {
        _localChange = maxVal;
      }
    }
    if (LCDML.BT_checkUp()) {
      LCDML.BT_resetUp();
      _localChange += change;
      if (_localChange > maxVal) {
        _localChange = minVal;
      }
    }
    if (LCDML.BT_checkEnter()) {
      _confirmedChangeIntVar = true;
      LCDML.FUNC_goBackToMenu();
    }
    lcd.setCursor(0, 1);
    String varToDisplay = String(_localChange * multiplier);
    lcd.print(appendSpace(varToDisplay));
  }

  if (LCDML.FUNC_close())     // ****** STABLE END *********
  {
    lcd.clear();
    if (*toChange != _localChange && _confirmedChangeIntVar) {
      *toChange = _localChange;
      lcd.setCursor(4, 0);
      lcd.print("GESPEICHERT!");
      lcd.setCursor(5, 1);
      lcd.print("Neuer Wert:");
      lcd.setCursor(8, 2);
      lcd.print(*toChange * multiplier);
      writeIntIntoEEPROM(address, *toChange);
    } else {
      lcd.setCursor(7, 1);
      lcd.print("KEINE");
      lcd.setCursor(5, 2);
      lcd.print("AENDERUNG");
    }
    _exitedChangeIntVar = true;
    _confirmedChangeIntVar = false;
    delay(READ_DELAY);
  }
}

void mFunc_setArbeitsbreite(uint8_t param) {
  changeIntVar("Arbeitsbreite", &arbeitsBreiteInDezimeter, 0, 240, 1, 0.1f, EPROM_ARBEITSBREITE, param);
}

void mFunc_setLiterProHektar(uint8_t param) {
  changeIntVar("Liter/Hektar", &literProHektar, 0, 1000, 1, 1, EPROM_LITER_PRO_HEKTAR, param);
}

void mFunc_setPulsesPerLiter(uint8_t param) {
  changeIntVar("Pulses/Liter", flussMesser.getPulsesPerUnitPointer(), 0, 1000, 1, 1, EPROM_FLUSSMESSER_PULSE_PER_LITER, param);
}

void mFunc_setPulsesPerMeter(uint8_t param) {
  changeIntVar("Pulses/Meter", traktorGeschwindigkeit.getPulsesPerUnitPointer(), 0, 1000, 1, 1, EPROM_TRAKTOR_PULSE_PER_METER, param);
}
void mFunc_setSimulatedSpeed(uint8_t param) {
  changeIntVar("Simul. Geschw.", &simulatedVelocity, 1, 55, 1, 1, EPROM_SIMULATED_SPEED, param);
}
void mFunc_setMaxLiterPerHour(uint8_t param) {
  changeIntVar("Pumpe max L/h", &maxLiterPerHour, 1, 2000, 1, 1, EPROM_MAX_LITER_PER_HOUR, param);
}
int _savedTraktorPulse = -1;
bool _exitedChangePulsePerMeterVelocity = true;
bool _confirmedChangePulsePerMeterVelocity = false;
void mFunc_setPulsesPerMeterVelocity(uint8_t param) {
  int minVal = 1;
  int maxVal = 1000;
  int change = 1;
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    if (_exitedChangePulsePerMeterVelocity) {
      _savedTraktorPulse = *traktorGeschwindigkeit.getPulsesPerUnitPointer();
      _exitedChangePulsePerMeterVelocity = false;
      _confirmedChangePulsePerMeterVelocity = false;
    }
    lcd.setCursor(0, 0);
    lcd.print("Geschwindigkeit"); // print some content on first row
    LCDML.FUNC_setLoopInterval(100);
  }


  if (LCDML.FUNC_loop())          // ****** LOOP *********
  {
    if (LCDML.BT_checkUp()) {
      LCDML.BT_resetUp();
      *traktorGeschwindigkeit.getPulsesPerUnitPointer() -= change;
      if (*traktorGeschwindigkeit.getPulsesPerUnitPointer() < minVal) {
        *traktorGeschwindigkeit.getPulsesPerUnitPointer() = minVal;
      }
    }
    if (LCDML.BT_checkDown()) {
      LCDML.BT_resetDown();
      *traktorGeschwindigkeit.getPulsesPerUnitPointer() += change;
      if (*traktorGeschwindigkeit.getPulsesPerUnitPointer() > maxVal) {
        *traktorGeschwindigkeit.getPulsesPerUnitPointer() = maxVal;
      }
    }
    if (LCDML.BT_checkEnter()) {
      _confirmedChangePulsePerMeterVelocity = true;
      LCDML.FUNC_goBackToMenu();
    }
    lcd.setCursor(0, 1);
    String ppu = String(*traktorGeschwindigkeit.getPulsesPerUnitPointer());
    lcd.print(appendSpace(String("Pulse: ") + ppu));
    lcd.setCursor(8, 1);
    lcd.setCursor(0, 2);
    String vel = String(traktorGeschwindigkeit.getValue() * 3.6f);
    lcd.print(appendSpace(String("Geschw: ") + vel));
  }

  if (LCDML.FUNC_close())     // ****** STABLE END *********
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
    delay(READ_DELAY);
  }

}

void mFunc_Verbrauch(uint8_t param) {
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    lcd.setCursor(1, 1);
    lcd.print(F("Verbrauch in Liter"));
    lcd.setCursor(6, 2);
    float liter = berechneVerbrauch();
    lcd.print(liter);
  }
  if (LCDML.FUNC_loop())
  {
    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      LCDML.FUNC_goBackToMenu();  // leave this function
    }
  }
}

long startedMillis = 0;
void mFunc_VerbrauchZuruecksetzen(uint8_t param) {
  if (LCDML.FUNC_setup())         // ****** SETUP *********
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
    delay(READ_DELAY);
    LCDML.FUNC_goBackToMenu();
  }
}

void driverMenu(uint8_t param) {
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    resetErrorState();
    lcd.clear();
    LCDML.FUNC_setLoopInterval(1000);
  }
  if (LCDML.FUNC_loop())          // ****** LOOP *********
  {
    driveMotor();
  }

  if (LCDML.FUNC_close())    // ****** STABLE END *********
  {
    analogWrite(PUMP_PWM_PIN, 0);
    turnPumpLedOff();
    turnSystemReadyLedOff();
    // you can here reset some global vars or do nothing
  }
}

void mFunc_startWithRealSpeed(uint8_t param) {
  useSimulatedVelocity = false;
  driverMenu(param);
}

void mFunc_startWithSimulatedSpeed(uint8_t param) {
  useSimulatedVelocity = true;
  driverMenu(param);
}
