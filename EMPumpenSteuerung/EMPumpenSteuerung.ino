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

//PINOUT
#define TRAKTOR_PULSE_PIN 2
#define FLOWMETER_PULSE_PIN 3
#define PUMP_PWM_PIN 5
#define WHITE_LED_PIN 4
#define GREEN_LED_PIN 6
#define RED_LED_PIN 7
//exit, enter, up, down pins are defined in LCDML_control

LiquidCrystal_I2C lcd(0x20/*0x27*/, _LCDML_DISP_cols, _LCDML_DISP_rows);
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
#define EPROM_ARBEITSBREITE 0 //2 byte
#define EPROM_LITER_PRO_HEKTAR 2 //2 byte
#define EPROM_TRAKTOR_PULSE_PER_METER 4 //2 byte
#define EPROM_FLUSSMESSER_PULSE_PER_LITER 6 //2 byte
#define EPROM_VERBRAUCH_GESAMMT 8 //4 byte
#define EPROM_SIMULATED_SPEED 12

bool useSimulatedVelocity = false;
int simulatedVelocity;
int arbeitsBreiteInDezimeter;
int literProHektar;
unsigned long summierterVerbrauch;

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
/*--*/LCDML_add(8, LCDML_0, 2, "Verbrauch", NULL);
/*--------*/LCDML_add(9, LCDML_0_2, 1, "Anzeigen", mFunc_Verbrauch);
/*--------*/LCDML_add(10, LCDML_0_2, 2, "Zuruecksetzen", mFunc_VerbrauchZuruecksetzen);
/*--*/LCDML_add(11, LCDML_0, 3, "Starten", mFunc_startWithRealSpeed);
/*--*/LCDML_add(12, LCDML_0, 4, "Simulierte Geschw.", NULL);
/*--------*/LCDML_add(13, LCDML_0_4, 1, "Geschw. anpassen", mFunc_setSimulatedSpeed);
/*--------*/LCDML_add(14, LCDML_0_4, 2, "Starten", mFunc_startWithSimulatedSpeed);

// menu element count - last element id
// this value must be the same as the last menu element
#define _LCDML_DISP_cnt    14

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

void writeUnsignedLongIntoEEPROM(int address, unsigned long number)
{
  EEPROM.update(address, number >> 24);
  EEPROM.update(address + 1, number >> 16);
  EEPROM.update(address + 2, number >> 8);
  EEPROM.update(address + 3, number & 0xFF);
}

unsigned long readUnsignedLongFromEEPROM(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  byte byte3 = EEPROM.read(address + 2);
  byte byte4 = EEPROM.read(address + 3);
  return ((unsigned long) byte1 << 24) + ((unsigned long) byte2 << 16) + ((unsigned long) byte3 << 8) + byte4;
}

void loadLastValuesFromEprom() {
  arbeitsBreiteInDezimeter = readIntFromEEPROM(EPROM_ARBEITSBREITE);
  literProHektar = readIntFromEEPROM(EPROM_LITER_PRO_HEKTAR);
  traktorGeschwindigkeit.setPulsesPerUnit(readIntFromEEPROM(EPROM_TRAKTOR_PULSE_PER_METER));
  flussMesser.setPulsesPerUnit(readIntFromEEPROM(EPROM_FLUSSMESSER_PULSE_PER_LITER));
  summierterVerbrauch = readUnsignedLongFromEEPROM(EPROM_VERBRAUCH_GESAMMT);
  simulatedVelocity = readIntFromEEPROM(EPROM_SIMULATED_SPEED);
}

void setup()
{
  loadLastValuesFromEprom();
  //used for measuring tractor and flow meter pulses
  attachInterrupt(digitalPinToInterrupt(TRAKTOR_PULSE_PIN), traktorPulseEnd, FALLING);
  attachInterrupt(digitalPinToInterrupt(FLOWMETER_PULSE_PIN), flussMesserPulseEnd, FALLING);
  pinMode(PUMP_PWM_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  analogWrite(6,64);
  // serial init; only be needed if serial control is used
  Serial.begin(9600);                // start serial
  Serial.println(F(_LCDML_VERSION)); // only for examples
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
int maxLiterPerHour = 800;
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
    Serial.println(pumpControlSignal);
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

void loop()
{
  LCDML.loop();
}
