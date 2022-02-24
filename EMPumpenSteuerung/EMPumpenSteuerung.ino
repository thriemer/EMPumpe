// ============================================================
// Example:     DisplayType - I2C LiquidCrystal
// ============================================================
// Description:
// This example includes the complete functionality over some
// tabs. All Tabs which are started with "LCDML_display_.."
// generates an output on the display / console / ....
// This example is for the author to test the complete functionality
// ============================================================
// *********************************************************************
// special settings
// *********************************************************************
// enable this line when you are not usigng a standard arduino
// for example when your chip is an ESP or a STM or SAM or something else

//#define _LCDML_cfg_use_ram

// *********************************************************************
// includes
// *********************************************************************
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LCDMenuLib2.h>
#include <PulseCounter.h>

// *********************************************************************
// LCDML display settings
// *********************************************************************
// settings for LCD
#define _LCDML_DISP_cols  20
#define _LCDML_DISP_rows  4

#define _LCDML_DISP_cfg_cursor                     0x7E   // cursor Symbol
#define _LCDML_DISP_cfg_scrollbar                  1      // enable a scrollbar

// LCD object
// for i2c there are many different steps for initialization, some are listed here
// when the rows and cols are not set here, they have to be set in the setup
//LiquidCrystal_I2C lcd(0x27);  // Set the LCD I2C address
//LiquidCrystal_I2C lcd(0x27, BACKLIGHT_PIN, POSITIVE);  // Set the LCD I2C address
LiquidCrystal_I2C lcd(0x20, _LCDML_DISP_cols, _LCDML_DISP_rows);
//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

const uint8_t scroll_bar[5][8] = {
  {B10001, B10001, B10001, B10001, B10001, B10001, B10001, B10001}, // scrollbar top
  {B11111, B11111, B10001, B10001, B10001, B10001, B10001, B10001}, // scroll state 1
  {B10001, B10001, B11111, B11111, B10001, B10001, B10001, B10001}, // scroll state 2
  {B10001, B10001, B10001, B10001, B11111, B11111, B10001, B10001}, // scroll state 3
  {B10001, B10001, B10001, B10001, B10001, B10001, B11111, B11111}  // scrollbar bottom
};

// *********************************************************************
// Prototypes
// *********************************************************************
void lcdml_menu_display();
void lcdml_menu_clear();
void lcdml_menu_control();

// *********************************************************************
// Global variables
// *********************************************************************


// *********************************************************************
// Objects
// *********************************************************************
LCDMenuLib2_menu LCDML_0 (255, 0, 0, NULL, NULL); // root menu element (do not change)
LCDMenuLib2 LCDML(LCDML_0, _LCDML_DISP_rows, _LCDML_DISP_cols, lcdml_menu_display, lcdml_menu_clear, lcdml_menu_control);

// *********************************************************************
// LCDML MENU/DISP
// *********************************************************************
// LCDML_0        => layer 0
// LCDML_0_X      => layer 1
// LCDML_0_X_X    => layer 2
// LCDML_0_X_X_X  => layer 3
// LCDML_0_...      => layer ...

// LCDML_add(id, prev_layer, new_num, lang_char_array, callback_function)

/*--*/LCDML_add(0, LCDML_0, 1, "Parameter", NULL);
/*--------*/LCDML_add(1, LCDML_0_1, 1, "Anzeigen", mFunc_showParameter);
/*--------*/LCDML_add(2, LCDML_0_1, 2, "Arbeitsbreite", mFunc_setArbeitsbreite);
/*--------*/LCDML_add(3, LCDML_0_1, 3, "L/ha", NULL);
/*--------*/LCDML_add(4, LCDML_0_1, 4, "Geschw. calibriern", NULL);
/*--------------*/LCDML_add(5, LCDML_0_1_4, 1, "Manuell pulse/sek", NULL);
/*--------------*/LCDML_add(6, LCDML_0_1_4, 2, "Per Messung", NULL);
/*--------*/LCDML_add(7, LCDML_0_1, 5, "Flusssensor", NULL);
/*--------------*/LCDML_add(8, LCDML_0_1_5, 1, "Manuell pulse/sek", NULL);
/*--------------*/LCDML_add(9, LCDML_0_1_5, 2, "Per Messung", NULL);
/*--*/LCDML_add(10, LCDML_0, 2, "Geschwindigkeit", mFunc_showVelocity);


// ***TIP*** Try to update _LCDML_DISP_cnt when you add a menu element.

// menu element count - last element id
// this value must be the same as the last menu element
#define _LCDML_DISP_cnt    10

// create menu
LCDML_createMenu(_LCDML_DISP_cnt);

// *********************************************************************
// SETUP
// *********************************************************************
//EMPumpenParameter
float arbeitsBreiten[] = {3, 5, 7, 11};
int arbeitsBreitenIndex;
float literProHektar;

PulseCounter traktorGeschwindigkeit;
PulseCounter flussMesser;

void traktorPulseEnd() {
  traktorGeschwindigkeit.pulseEnded();
}
void flussMesserPulseEnd() {
  flussMesser.pulseEnded();
}
void setup()
{
  attachInterrupt(digitalPinToInterrupt(2), traktorPulseEnd, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), flussMesserPulseEnd, FALLING);
  
  // serial init; only be needed if serial control is used
  Serial.begin(9600);                // start serial
  Serial.println(F(_LCDML_VERSION)); // only for examples
  analogWrite(10, 127);


  // LCD Begin
  lcd.init();
  lcd.backlight();
  //lcd.begin(_LCDML_DISP_cols,_LCDML_DISP_rows);  // some display types needs here the initialization


  // set special chars for scrollbar
  lcd.createChar(0, (uint8_t*)scroll_bar[0]);
  lcd.createChar(1, (uint8_t*)scroll_bar[1]);
  lcd.createChar(2, (uint8_t*)scroll_bar[2]);
  lcd.createChar(3, (uint8_t*)scroll_bar[3]);
  lcd.createChar(4, (uint8_t*)scroll_bar[4]);

  // LCDMenuLib Setup
  LCDML_setup(_LCDML_DISP_cnt);

  // Some settings which can be used

  // Enable Menu Rollover
  LCDML.MENU_enRollover();

  // Enable Screensaver (screensaver menu function, time to activate in ms)
  //LCDML.SCREEN_enable(mFunc_screensaver, 10000); // set to 10 seconds
  LCDML.SCREEN_disable();

  // Some needful methods

  // You can jump to a menu function from anywhere with
  //LCDML.OTHER_jumpToFunc(mFunc_p2); // the parameter is the function name
}

// *********************************************************************
// LOOP
// *********************************************************************
void loop()
{
  LCDML.loop();
}
