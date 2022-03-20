#define CLEAR_ROW "                    "
/* ===================================================================== *
 *                                                                       *
   Menu Callback Function
 *                                                                       *
   =====================================================================

   EXAMPLE CODE:

  // *********************************************************************
  void your_function_name(uint8_t param)
  // *********************************************************************
  {
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    //LCDML_UNUSED(param);
    // setup
    // is called only if it is started

    // starts a trigger event for the loop function every 100 milliseconds
    LCDML.FUNC_setLoopInterval(100);

    // uncomment this line when the menu should go back to the last called position
    // this could be a cursor position or the an active menu function
    // GBA means => go back advanced
    //LCDML.FUNC_setGBA()

    //
  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop
    // is called when it is triggered
    // - with LCDML_DISP_triggerMenu( milliseconds )
    // - with every button or event status change

    // uncomment this line when the screensaver should not be called when this function is running
    // reset screensaver timer
    //LCDML.SCREEN_resetTimer();

    // check if any button is pressed (enter, up, down, left, right)
    if(LCDML.BT_checkAny()) {
      LCDML.FUNC_goBackToMenu();
    }
  }

  if(LCDML.FUNC_close())      // ****** STABLE END *********
  {
    // loop end
    // you can here reset some global vars or delete it
    // this function is always called when the functions ends.
    // this means when you are calling a jumpTo ore a goRoot function
    // that this part is called before a function is closed
  }
  }


   =====================================================================
*/


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
    lcd.print(arbeitsBreiten[arbeitsBreitenIndex]);
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

int _localChange = -1;
bool _exitedChangeIntVar = true;
bool _confirmedChangeIntVar = false;
void changeIntVar(String varName, int* toChange, int minVal, int maxVal, int change, bool saveValue, int address, uint8_t param) {
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
    lcd.setCursor(0,1);
    lcd.print(CLEAR_ROW);
    lcd.setCursor(0, 1);
    lcd.print(_localChange);
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
      lcd.print(*toChange);
      if (saveValue) {
        writeIntIntoEEPROM(address, *toChange);
      }
    } else {
      lcd.setCursor(7, 1);
      lcd.print("KEINE");
      lcd.setCursor(5, 2);
      lcd.print("AENDERUNG");
    }
    _exitedChangeIntVar = true;
    _confirmedChangeIntVar = false;
    delay(2000);
  }
}

void mFunc_setArbeitsbreite(uint8_t param) {
  changeIntVar("Arbeitsbreite", &arbeitsBreitenIndex, 0, 3, 1,true,EPROM_ARBEITSBREITE, param);
}

void mFunc_setLiterProHektar(uint8_t param) {
  changeIntVar("Liter/Hektar", &literProHektar, 0, 1000, 10,true,EPROM_LITER_PRO_HEKTAR, param);
}

void mFunc_setPulsesPerLiter(uint8_t param) {
  changeIntVar("Pulses/Liter", flussMesser.getPulsesPerUnitPointer(), 0, 1000, 1,true,EPROM_FLUSSMESSER_PULSE_PER_LITER, param);
}

void mFunc_setPulsesPerMeter(uint8_t param) {
  changeIntVar("Pulses/Meter", traktorGeschwindigkeit.getPulsesPerUnitPointer(), 0, 1000, 1,true,EPROM_TRAKTOR_PULSE_PER_METER, param);
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
    lcd.print(CLEAR_ROW);//clear row
    lcd.setCursor(0, 1);
    lcd.print("Pulse:");
    lcd.setCursor(8, 1);
    lcd.print(*traktorGeschwindigkeit.getPulsesPerUnitPointer());
    lcd.setCursor(0, 2);
    lcd.print(CLEAR_ROW);//clear row
    lcd.setCursor(0, 2);
    lcd.print("Geschw:");
    lcd.setCursor(8, 2);
    lcd.print(traktorGeschwindigkeit.getValue() * 3.6f);
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
      writeIntIntoEEPROM(EPROM_TRAKTOR_PULSE_PER_METER,*traktorGeschwindigkeit.getPulsesPerUnitPointer());
    } else {
      *traktorGeschwindigkeit.getPulsesPerUnitPointer() = _savedTraktorPulse;
      lcd.setCursor(7, 1);
      lcd.print("KEINE");
      lcd.setCursor(5, 2);
      lcd.print("AENDERUNG");
    }
    _exitedChangePulsePerMeterVelocity = true;
    _confirmedChangePulsePerMeterVelocity = false;
    delay(2000);
  }

}

void mFunc_showVelocity(uint8_t param) {
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // setup function
    // print LCD content
    // print LCD content
    lcd.setCursor(0, 0);
    lcd.print(F("Geschwindigkeit "));
    LCDML.FUNC_setLoopInterval(100);
  }

  if (LCDML.FUNC_loop())          // ****** LOOP *********
  {
    lcd.setCursor(0, 1);
    lcd.print(traktorGeschwindigkeit.getValue());
    lcd.setCursor(10, 1);
    lcd.print("m/s");

    lcd.setCursor(0, 2);
    lcd.print(traktorGeschwindigkeit.getValue() * 3.6f);
    lcd.setCursor(10, 2);
    lcd.print("km/h");

  }

  if (LCDML.FUNC_close())    // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}



// *********************************************************************
void mFunc_screensaver(uint8_t param)
// *********************************************************************
{
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // update LCD content
    lcd.setCursor(0, 0); // set cursor
    lcd.print(F("screensaver")); // print change content
    lcd.setCursor(0, 1); // set cursor
    lcd.print(F("press any key"));
    lcd.setCursor(0, 2); // set cursor
    lcd.print(F("to leave it"));
    LCDML.FUNC_setLoopInterval(100);  // starts a trigger event for the loop function every 100 milliseconds
  }

  if (LCDML.FUNC_loop())
  {
    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      LCDML.FUNC_goBackToMenu();  // leave this function
    }
  }

  if (LCDML.FUNC_close())
  {
    // The screensaver go to the root menu
    LCDML.MENU_goRoot();
  }
}



// *********************************************************************
void mFunc_back(uint8_t param)
// *********************************************************************
{
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // end function and go an layer back
    LCDML.FUNC_goBackToMenu(1);      // leave this function and go a layer back
  }
}


// *********************************************************************
void mFunc_goToRootMenu(uint8_t param)
// *********************************************************************
{
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // go to root and display menu
    LCDML.MENU_goRoot();
  }
}


// *********************************************************************
void mFunc_jumpTo_timer_info(uint8_t param)
// *********************************************************************
{
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // Jump to main screen
  }
}
