#define EPROM_ARBEITSBREITE 0 //2 byte
#define EPROM_LITER_PRO_HEKTAR 2 //2 byte
#define EPROM_TRAKTOR_PULSE_PER_METER 4 //2 byte
#define EPROM_FLUSSMESSER_PULSE_PER_LITER 6 //2 byte
#define EPROM_VERBRAUCH_GESAMMT 8 //4 byte
#define EPROM_SIMULATED_SPEED 12 //2 byte
#define EPROM_MAX_LITER_PER_HOUR 14

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
  maxLiterPerHour = readIntFromEEPROM(EPROM_MAX_LITER_PER_HOUR);
}
