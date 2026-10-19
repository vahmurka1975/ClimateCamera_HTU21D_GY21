#ifndef SHT30_RELIABLE_H
#define SHT30_RELIABLE_H
#include <Arduino.h>
#include <Wire.h>

// HTU21D / SHT21 / Si7021 (модуль GY-21): адрес 0x40, режим No-Hold Master
// (0xF3 — запуск измерения температуры 14 бит, 0xF5 — запуск измерения влажности 12 бит).
// Программный Open-Drain I2C (bit-banging) без зависимости от буферов Wire,
// оптимизированный для длинного кабеля (до 150 см) на пинах SDA=21, SCL=22.
void initSHT30();
bool readSHT30(float &outTemp, float &outHum);

class Sht30Reliable {
 public:
  enum class Error : uint8_t {
    None, I2cBegin, ClockLow, DataLow, WriteCommand, EndTransmission,
    ShortRead, TemperatureCrc, HumidityCrc, BothCrc, StatusCrc,
    StatusFlags, ValueOutOfRange
  };
  struct Diagnostic {
    Error error=Error::None;
    uint8_t wireCode=0, received=0, expected=0;
    uint8_t crcReceived=0, crcExpected=0;
    uint8_t humCrcReceived=0, humCrcExpected=0;
    uint16_t command=0, status=0;
  };
  Sht30Reliable(TwoWire &wire,uint8_t sda,uint8_t scl,uint8_t address=0x40);
  bool begin();
  bool recover();
  bool startMeasurement();
  bool readMeasurement(float &temperature,float &humidity);
  const Diagnostic &diagnostic() const { return diag; }
  const char *errorName() const;
  void formatError(char *out,size_t length) const;
  static uint8_t crc8(const uint8_t *data,size_t length);
 private:
  TwoWire &bus;
  uint8_t pinSda,pinScl,addr;
  Diagnostic diag;
  void sdaHigh() const;
  void sdaLow() const;
  void sclHigh() const;
  void sclLow() const;
  void i2cStart() const;
  void i2cStop() const;
  bool i2cWriteByte(uint8_t data) const;
  uint8_t i2cReadByte(bool ack) const;
  bool command(uint8_t value);
  bool readBytes(uint8_t *data,size_t length,uint32_t maxWaitMs=95);
  bool waitClockHigh();
};
#endif
