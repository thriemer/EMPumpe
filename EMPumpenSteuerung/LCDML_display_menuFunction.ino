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
    lcd.setCursor(0, 1);
    lcd.print(arbeitsBreiten[arbeitsBreitenIndex]);
    lcd.setCursor(0, 2);
    lcd.print(F("Liter pro Hektar:"));
    lcd.setCursor(0, 3);
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

int _arbeitsBreitenIndex = 0;
// *********************************************************************
void mFunc_setArbeitsbreite(uint8_t param)
// *********************************************************************
{
  if (LCDML.FUNC_setup())         // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    _arbeitsBreitenIndex = arbeitsBreitenIndex;
    lcd.setCursor(0, 0);
    lcd.print(F("Arbeitsbreite")); // print some content on first row
  }


  if (LCDML.FUNC_loop())          // ****** LOOP *********
  {
    if (LCDML.BT_checkLeft()) {
      LCDML.BT_resetLeft();
      _arbeitsBreitenIndex--;
      if (_arbeitsBreitenIndex < 0) {
        _arbeitsBreitenIndex = 3;
      }
    }
    if (LCDML.BT_checkRight()) {
      LCDML.BT_resetRight();
      _arbeitsBreitenIndex++;
      if (_arbeitsBreitenIndex > 3) {
        _arbeitsBreitenIndex = 0;
      }
    }
    if (LCDML.BT_checkEnter()) {
      arbeitsBreitenIndex = _arbeitsBreitenIndex;
      LCDML.FUNC_goBackToMenu();
    }
    lcd.setCursor(0, 1);
    lcd.print(arbeitsBreiten[_arbeitsBreitenIndex]);
  }

  if (LCDML.FUNC_close())     // ****** STABLE END *********
  {
  }
}


// *********************************************************************
void mFunc_showVelocity(uint8_t param)
// *********************************************************************
{
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
