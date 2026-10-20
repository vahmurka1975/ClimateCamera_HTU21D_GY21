#ifndef HTU21D_RELIABLE_H
#define HTU21D_RELIABLE_H
#include <Arduino.h>
#include <Wire.h>

// Драйвер HTU21D / HTU21D / Si7021 (модуль GY-211, адрес 0x40)
// Работает через аппаратный контроллер TwoWire (Wire) на пинах SDA=21, SCL=22,
// в точности повторяя тайминги и команды рабочего тестового скетча.
void initHTU21D();
bool readHTU21D(float &outTemp, float &outHum);

class Htu21dReliable {
 public:
  enum class Error : uint8_t {
    None, I2cBegin, ClockLow, DataLow, WriteCommand, EndTransmission,
    ShortRead, TemperatureCrc, HumidityCrc, BothCrc, StatusCrc,
    StatusFlags, ValueOutOfRange
  };
  struct Diagnostic {
    Error error = Error::None;
    uint8_t wireCode = 0, received = 0, expected = 0;
    uint8_t crcReceived = 0, crcExpected = 0;
    uint8_t humCrcReceived = 0, humCrcExpected = 0;
    uint16_t command = 0, status = 0;
  };

  Htu21dReliable(TwoWire &wire, uint8_t sda, uint8_t scl, uint8_t address = 0x40);
  bool begin();
  bool recover();
  bool startMeasurement();
  bool readMeasurement(float &temperature, float &humidity);

  const Diagnostic &diagnostic() const { return diag; }
  const char *errorName() const;
  void formatError(char *out, size_t length) const;
  static uint8_t crc8(const uint8_t *data, size_t length);

 private:
  TwoWire &bus;
  uint8_t pinSda, pinScl, addr;
  Diagnostic diag;

  bool command(uint8_t value);
  bool readBytes(uint8_t *data, size_t length, uint32_t maxWaitMs = 95);
  bool waitClockHigh();
};
#endif