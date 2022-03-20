#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LCDMenuLib2.h>
#include <PulseCounter.h>
#include <EEPROM.h>

// *********************************************************************
// LCDML display settings
// *********************************************************************
// settings for LCD
#define _LCDML_DISP_cols  20
#define _LCDML_DISP_rows  4

#define _LCDML_DISP_cfg_cursor                     0x7E   // cursor Symbol
#define _LCDML_DISP_cfg_scrollbar                  1      // enable a scrollbar

LiquidCrystal_I2C lcd(0x20, _LCDML_DISP_cols, _LCDML_DISP_rows);
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
#define EPROM_ARBEITSBREITE 0
#define EPROM_LITER_PRO_HEKTAR 2
#define EPROM_TRAKTOR_PULSE_PER_METER 4
#define EPROM_FLUSSMESSER_PULSE_PER_LITER 6

float arbeitsBreiten[] = {3, 5, 7, 11};
int arbeitsBreitenIndex;
int literProHektar;

PulseCounter traktorGeschwindigkeit;
PulseCounter flussMesser;

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
/*--------*/LCDML_add(3, LCDML_0_1, 3, "Liter/Hektar", mFunc_setLiterProHektar);
/*--------*/LCDML_add(4, LCDML_0_1, 4, "Geschw. calibriern", NULL);
/*--------------*/LCDML_add(5, LCDML_0_1_4, 1, "Manuell pulse/sek", mFunc_setPulsesPerMeter);
/*--------------*/LCDML_add(6, LCDML_0_1_4, 2, "Per Messung", mFunc_setPulsesPerMeterVelocity);
/*--------*/LCDML_add(7, LCDML_0_1, 5, "Flusssensor p/s", mFunc_setPulsesPerLiter);
/*--*/LCDML_add(8, LCDML_0, 2, "Geschwindigkeit", mFunc_showVelocity);

// menu element count - last element id
// this value must be the same as the last menu element
#define _LCDML_DISP_cnt    8

// create menu
LCDML_createMenu(_LCDML_DISP_cnt);

// *********************************************************************
// SETUP
// *********************************************************************

void traktorPulseEnd() {
  traktorGeschwindigkeit.pulseEnded();
}
void flussMesserPulseEnd() {
  flussMesser.pulseEnded();
}

void writeIntIntoEEPROM(int address, int number)
{
  EEPROM.update(address, number >> 8);
  EEPROM.update(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

void loadLastValuesFromEprom() {
  arbeitsBreitenIndex = readIntFromEEPROM(EPROM_ARBEITSBREITE);
  literProHektar = readIntFromEEPROM(EPROM_LITER_PRO_HEKTAR);
  traktorGeschwindigkeit.setPulsesPerUnit(readIntFromEEPROM(EPROM_TRAKTOR_PULSE_PER_METER));
  flussMesser.setPulsesPerUnit(readIntFromEEPROM(EPROM_FLUSSMESSER_PULSE_PER_LITER));
}

void setup()
{
  loadLastValuesFromEprom();
  //used for measuring tractor and flow meter pulses
  attachInterrupt(digitalPinToInterrupt(2), traktorPulseEnd, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), flussMesserPulseEnd, FALLING);

  // serial init; only be needed if serial control is used
  Serial.begin(9600);                // start serial
  Serial.println(F(_LCDML_VERSION)); // only for examples
  analogWrite(10, 127);
  Serial.print("Size of Char: ");
  Serial.println(sizeof(char));
  Serial.print("Size of Bool: ");
  Serial.println(sizeof(bool));

  Serial.print("Size of Int: ");
  Serial.println(sizeof(int));
  Serial.print("Size of Unsigned Int: ");
  Serial.println(sizeof(unsigned int));
  Serial.print("Size of Long: ");
  Serial.println(sizeof(long));
  Serial.print("Size of Float: ");
  Serial.println(sizeof(float));
  Serial.print("Size of Double: ");
  Serial.println(sizeof(double));
  // LCD Begin
  lcd.init();
  lcd.backlight();

  // set special chars for scrollbar
  lcd.createChar(0, (uint8_t*)scroll_bar[0]);
  lcd.createChar(1, (uint8_t*)scroll_bar[1]);
  lcd.createChar(2, (uint8_t*)scroll_bar[2]);
  lcd.createChar(3, (uint8_t*)scroll_bar[3]);
  lcd.createChar(4, (uint8_t*)scroll_bar[4]);

  // LCDMenuLib Setup
  LCDML_setup(_LCDML_DISP_cnt);

  // Enable Menu Rollover
  LCDML.MENU_enRollover();
  // disable screensaver
  LCDML.SCREEN_disable();
}

void loop()
{
  LCDML.loop();
}
