#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LCDMenuLib2.h>
#include <PulseCounter.h>
#include <EEPROM.h>
#include <PID_v1.h>

// *********************************************************************
// LCDML display settings
// *********************************************************************
// settings for LCD
#define _LCDML_DISP_cols  20
#define _LCDML_DISP_rows  4

#define _LCDML_DISP_cfg_cursor                     0x7E   // cursor Symbol
#define _LCDML_DISP_cfg_scrollbar                  1      // enable a scrollbar

//PINOUT
#define TRAKTOR_PULSE_PIN 2
#define FLOWMETER_PULSE_PIN 3
#define PUMP_PWM_PIN 5
#define WHITE_LED_PIN 4
#define GREEN_LED_PIN 6
#define RED_LED_PIN 7
//exit, enter, up, down pins are defined in LCDML_control

LiquidCrystal_I2C lcd(0x27/*0x27*/, _LCDML_DISP_cols, _LCDML_DISP_rows);
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

bool useSimulatedVelocity = false;
int simulatedVelocity;
int maxLiterPerHour;
int arbeitsBreiteInDezimeter;
int literProHektar;
unsigned long summierterVerbrauch;
int pidProportional;
int pidIntegral;
int pidDerivative;

PulseCounter traktorGeschwindigkeit;
PulseCounter flussMesser;
double pumpSetPoint, flowSensorReading, pumpControl;
PID pumpPID(&flowSensorReading, &pumpControl, &pumpSetPoint, 0.1, 1, 0.1, DIRECT);

void setPIDValues(){
    pumpPID.SetTunings(pidProportional/10.0, pidIntegral/10.0, pidDerivative/10.0);
    pumpPID.SetOutputLimits(0, maxLiterPerHour/(60 * 60.0d));
}

bool repeatedDeviating, moreFlowThanPossible;

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
/*--------*/LCDML_add(7, LCDML_0_1, 5, "PID calibriern", NULL);
/*--------------*/LCDML_add(8, LCDML_0_1_5, 1, "Proportional", mFunc_setPIDProportional);
/*--------------*/LCDML_add(9, LCDML_0_1_5, 2, "Integral", mFunc_setPIDIntegral);
/*--------------*/LCDML_add(10, LCDML_0_1_5, 3, "Ableitung", mFunc_setPIDDerivative);
/*--------*/LCDML_add(11, LCDML_0_1, 6, "Flusssensor p/s", mFunc_setPulsesPerLiter);
/*--------*/LCDML_add(12, LCDML_0_1, 7, "Pumpe max L/h", mFunc_setMaxLiterPerHour);
/*--*/LCDML_add(13, LCDML_0, 2, "Verbrauch", NULL);
/*--------*/LCDML_add(14, LCDML_0_2, 1, "Anzeigen", mFunc_Verbrauch);
/*--------*/LCDML_add(15, LCDML_0_2, 2, "Zuruecksetzen", mFunc_VerbrauchZuruecksetzen);
/*--*/LCDML_add(16, LCDML_0, 3, "Starten", mFunc_startWithRealSpeed);
/*--*/LCDML_add(17, LCDML_0, 4, "Simulierte Geschw.", NULL);
/*--------*/LCDML_add(18, LCDML_0_4, 1, "Geschw. anpassen", mFunc_setSimulatedSpeed);
/*--------*/LCDML_add(19, LCDML_0_4, 2, "Starten", mFunc_startWithSimulatedSpeed);

// menu element count - last element id
// this value must be the same as the last menu element
#define _LCDML_DISP_cnt    19

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

void setup()
{
  Serial.begin(9600);
  loadLastValuesFromEprom();
  //used for measuring tractor and flow meter pulses
  attachInterrupt(digitalPinToInterrupt(TRAKTOR_PULSE_PIN), traktorPulseEnd, FALLING);
  attachInterrupt(digitalPinToInterrupt(FLOWMETER_PULSE_PIN), flussMesserPulseEnd, FALLING);
  pinMode(PUMP_PWM_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pumpPID.SetMode(AUTOMATIC);
  pumpPID.SetSampleTime(990);
  // serial init; only be needed if serial control is used
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
  controlRedLED();
  LCDML.loop();
}
