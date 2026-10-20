#include "Htu21dReliable.h"
#include <math.h>
#include <stdio.h>

// ===================== АППАРАТНЫЙ I2C (Wire) ДРАЙВЕР HTU21D / HTU21D / Si7021 (GY-211) =====================
// Полностью соответствует логике рабочего тестового скетча:
// SDA = GPIO 21, SCL = GPIO 22, адрес = 0x40, команды No-Hold Master 0xF3 (T) и 0xF5 (RH).
static constexpr uint8_t  HTU21_DEFAULT_SDA  = 21;
static constexpr uint8_t  HTU21_DEFAULT_SCL  = 22;
static constexpr uint8_t  HTU21_DEFAULT_ADDR = 0x40;
static constexpr uint32_t HTU21_I2C_FREQ_HZ  = 100000UL; // 100 кГц (как в рабочем тесте)

static constexpr uint8_t CMD_TRIG_TEMP_NOHOLD = 0xF3;
static constexpr uint8_t CMD_TRIG_HUM_NOHOLD  = 0xF5;
static constexpr uint8_t CMD_SOFT_RESET       = 0xFE;

// CRC-8 для HTU21D / HTU21D / Si7021 (полином 0x31, начальное значение 0x00)
uint8_t htu21dCrc8(const uint8_t *data, size_t len) {
  uint8_t crc = 0x00;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; ++b) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}

void initHTU21D() {
  Wire.end();
  pinMode(HTU21_DEFAULT_SDA, INPUT_PULLUP);
  pinMode(HTU21_DEFAULT_SCL, INPUT_PULLUP);
  delay(2);
  Wire.begin(HTU21_DEFAULT_SDA, HTU21_DEFAULT_SCL, HTU21_I2C_FREQ_HZ);
  Wire.setClock(HTU21_I2C_FREQ_HZ);
  Wire.setTimeOut(50);

  Wire.beginTransmission(HTU21_DEFAULT_ADDR);
  Wire.write(CMD_SOFT_RESET);
  Wire.endTransmission(true);
  delay(20);
}

static bool wireRead3WithPoll(TwoWire &bus, uint8_t addr, uint8_t out[3], uint32_t maxWaitMs, uint8_t &gotOut) {
  gotOut = 0;
  uint32_t t0 = millis();
  for (;;) {
    uint8_t got = (uint8_t)bus.requestFrom((uint8_t)addr, (uint8_t)3, (uint8_t)true);
    if (got == 3) {
      out[0] = (uint8_t)bus.read();
      out[1] = (uint8_t)bus.read();
      out[2] = (uint8_t)bus.read();
      gotOut = 3;
      return true;
    }
    while (bus.available()) (void)bus.read();
    if ((uint32_t)(millis() - t0) >= maxWaitMs) break;
    delay(4);
  }
  return false;
}

bool readHTU21D(float &outTemp, float &outHum) {
  Wire.beginTransmission(HTU21_DEFAULT_ADDR);
  Wire.write(CMD_TRIG_TEMP_NOHOLD);
  if (Wire.endTransmission(true) != 0) {
    initHTU21D();
    Wire.beginTransmission(HTU21_DEFAULT_ADDR);
    Wire.write(CMD_TRIG_TEMP_NOHOLD);
    if (Wire.endTransmission(true) != 0) return false;
  }

  delay(65);
  uint8_t tb[3] = {0};
  uint8_t got = 0;
  if (!wireRead3WithPoll(Wire, HTU21_DEFAULT_ADDR, tb, 60, got) || htu21dCrc8(tb, 2) != tb[2]) {
    return false;
  }

  Wire.beginTransmission(HTU21_DEFAULT_ADDR);
  Wire.write(CMD_TRIG_HUM_NOHOLD);
  if (Wire.endTransmission(true) != 0) return false;

  delay(35);
  uint8_t hb[3] = {0};
  if (!wireRead3WithPoll(Wire, HTU21_DEFAULT_ADDR, hb, 60, got) || htu21dCrc8(hb, 2) != hb[2]) {
    return false;
  }

  uint16_t rawT = (((uint16_t)tb[0] << 8) | tb[1]) & 0xFFFC;
  uint16_t rawH = (((uint16_t)hb[0] << 8) | hb[1]) & 0xFFFC;
  outTemp = -46.85f + 175.72f * ((float)rawT / 65536.0f);
  float h = -6.0f   + 125.0f  * ((float)rawH / 65536.0f);
  if (h < 0.0f && h >= -6.0f) h = 0.0f;
  if (h > 100.0f && h <= 119.0f) h = 100.0f;
  outHum = h;
  return true;
}

// =========================================================================================================

Htu21dReliable::Htu21dReliable(TwoWire &wire, uint8_t sda, uint8_t scl, uint8_t address)
  : bus(wire), pinSda(sda), pinScl(scl), addr(address) {}

uint8_t Htu21dReliable::crc8(const uint8_t *data, size_t length) {
  return htu21dCrc8(data, length);
}

bool Htu21dReliable::waitClockHigh() {
  uint32_t start = micros();
  while (digitalRead(pinScl) == LOW) {
    if ((uint32_t)(micros() - start) >= 2500UL) return false;
    delayMicroseconds(5);
  }
  return true;
}

bool Htu21dReliable::command(uint8_t value) {
  diag.command = value;
  diag.wireCode = 0;

  bus.beginTransmission(addr);
  if (bus.write(value) != 1) {
    (void)bus.endTransmission(true);
    diag.error = Error::WriteCommand;
    return false;
  }
  uint8_t err = bus.endTransmission(true);
  diag.wireCode = err;
  if (err != 0) {
    diag.error = Error::EndTransmission;
    return false;
  }
  return true;
}

bool Htu21dReliable::readBytes(uint8_t *data, size_t length, uint32_t maxWaitMs) {
  diag.expected = (uint8_t)length;
  diag.received = 0;

  uint32_t t0 = millis();
  for (;;) {
    uint8_t got = (uint8_t)bus.requestFrom((uint8_t)addr, (uint8_t)length, (uint8_t)true);
    if (got == length) {
      for (size_t i = 0; i < length; ++i) {
        data[i] = (uint8_t)bus.read();
      }
      diag.received = got;
      return true;
    }
    while (bus.available()) (void)bus.read();
    if ((uint32_t)(millis() - t0) >= maxWaitMs) break;
    delay(4);
  }

  diag.error = Error::ShortRead;
  return false;
}

bool Htu21dReliable::begin() {
  return recover();
}

bool Htu21dReliable::recover() {
  diag = Diagnostic();

  // 1. Освобождаем шину в режиме Open-Drain, если ведомый удерживает SDA после прерванного чтения
  bus.end();
  pinMode(pinSda, INPUT_PULLUP);
  pinMode(pinScl, INPUT_PULLUP);
  delay(2);

  if (!waitClockHigh()) {
    diag.error = Error::ClockLow;
    return false;
  }

  if (digitalRead(pinSda) == LOW) {
    digitalWrite(pinSda, HIGH);
    digitalWrite(pinScl, HIGH);
    pinMode(pinSda, OUTPUT_OPEN_DRAIN);
    pinMode(pinScl, OUTPUT_OPEN_DRAIN);
    for (uint8_t pulse = 0; pulse < 9 && digitalRead(pinSda) == LOW; ++pulse) {
      digitalWrite(pinScl, LOW);
      delayMicroseconds(10);
      digitalWrite(pinScl, HIGH);
      if (!waitClockHigh()) {
        diag.error = Error::ClockLow;
        return false;
      }
      delayMicroseconds(10);
    }
    // Формируем STOP
    digitalWrite(pinScl, LOW);
    digitalWrite(pinSda, LOW);
    delayMicroseconds(10);
    digitalWrite(pinScl, HIGH);
    delayMicroseconds(10);
    digitalWrite(pinSda, HIGH);
    delayMicroseconds(10);
  }

  pinMode(pinSda, INPUT_PULLUP);
  pinMode(pinScl, INPUT_PULLUP);
  delay(1);

  if (digitalRead(pinScl) == LOW) {
    diag.error = Error::ClockLow;
    return false;
  }
  if (digitalRead(pinSda) == LOW) {
    diag.error = Error::DataLow;
    return false;
  }

  // 2. Запускаем аппаратный контроллер I2C (Wire) на пинах 21/22 (100 кГц, как в рабочем тесте)
  if (!bus.begin(pinSda, pinScl, HTU21_I2C_FREQ_HZ)) {
    diag.error = Error::I2cBegin;
    return false;
  }
  bus.setClock(HTU21_I2C_FREQ_HZ);
  bus.setTimeOut(50);
  delay(5);

  // 3. Отправляем Soft Reset (0xFE), как в рабочем тестовом скетче, и ждём 20 мс
  bus.beginTransmission(addr);
  bus.write(CMD_SOFT_RESET);
  (void)bus.endTransmission(true);
  delay(20);

  // 4. Проверяем присутствие датчика на адресе 0x40 (либо ответом на пустую транзакцию, либо запуском 0xF3)
  bus.beginTransmission(addr);
  uint8_t err = bus.endTransmission(true);
  if (err != 0) {
    // Повторная попытка на пониженной частоте 50 кГц для длинного кабеля
    bus.setClock(50000UL);
    delay(5);
    bus.beginTransmission(addr);
    err = bus.endTransmission(true);
    if (err != 0) {
      diag.command = CMD_SOFT_RESET;
      diag.wireCode = err;
      diag.error = Error::EndTransmission;
      return false;
    }
  }

  diag = Diagnostic();
  return true;
}

bool Htu21dReliable::startMeasurement() {
  diag = Diagnostic();
  if (!command(CMD_TRIG_TEMP_NOHOLD)) {
    if (!recover()) return false;
    if (!command(CMD_TRIG_TEMP_NOHOLD)) return false;
  }
  return true;
}

bool Htu21dReliable::readMeasurement(float &temperature, float &humidity) {
  diag = Diagnostic();

  // 1. Чтение температуры (3 байта: MSB, LSB, CRC) после неблокирующей паузы SENSOR_CONVERSION_MS
  uint8_t tBytes[3] = {0};
  if (!readBytes(tBytes, sizeof(tBytes), 65)) {
    return false;
  }
  diag.crcExpected = crc8(tBytes, 2);
  diag.crcReceived = tBytes[2];
  if (diag.crcExpected != diag.crcReceived) {
    diag.error = Error::TemperatureCrc;
    return false;
  }
  uint16_t rawT = (((uint16_t)tBytes[0] << 8) | tBytes[1]) & 0xFFFC;

  // 2. Запуск измерения влажности No-Hold Master (0xF5) и чтение 3 байт (MSB, LSB, CRC)
  if (!command(CMD_TRIG_HUM_NOHOLD)) {
    return false;
  }
  delay(35);
  uint8_t hBytes[3] = {0};
  if (!readBytes(hBytes, sizeof(hBytes), 65)) {
    return false;
  }
  diag.humCrcExpected = crc8(hBytes, 2);
  diag.humCrcReceived = hBytes[2];
  if (diag.humCrcExpected != diag.humCrcReceived) {
    diag.error = Error::HumidityCrc;
    return false;
  }
  uint16_t rawH = (((uint16_t)hBytes[0] << 8) | hBytes[1]) & 0xFFFC;

  // 3. Пересчёт по формулам даташита HTU21D / HTU21D / Si7021
  float t = -46.85f + 175.72f * ((float)rawT / 65536.0f);
  float h = -6.0f   + 125.0f  * ((float)rawH / 65536.0f);
  if (h < 0.0f && h >= -6.0f) h = 0.0f;
  if (h > 100.0f && h <= 119.0f) h = 100.0f;

  if (!isfinite(t) || !isfinite(h) || t < -40.0f || t > 125.0f || h < 0.0f || h > 100.0f) {
    diag.error = Error::ValueOutOfRange;
    return false;
  }

  temperature = t;
  humidity = h;
  return true;
}

const char *Htu21dReliable::errorName() const {
  switch (diag.error) {
    case Error::None:            return "none";
    case Error::I2cBegin:        return "i2c_begin";
    case Error::ClockLow:        return "scl_stuck_low";
    case Error::DataLow:         return "sda_stuck_low";
    case Error::WriteCommand:    return "command_write";
    case Error::EndTransmission: return "end_transmission";
    case Error::ShortRead:       return "short_read";
    case Error::TemperatureCrc:  return "temperature_crc";
    case Error::HumidityCrc:     return "humidity_crc";
    case Error::BothCrc:         return "temperature_and_humidity_crc";
    case Error::StatusCrc:       return "status_crc";
    case Error::StatusFlags:     return "status_flags";
    case Error::ValueOutOfRange: return "value_out_of_range";
  }
  return "unknown";
}

void Htu21dReliable::formatError(char *out, size_t length) const {
  if (!out || !length) return;
  switch (diag.error) {
    case Error::None:
      snprintf(out, length, "нет ошибки");
      break;
    case Error::I2cBegin:
      snprintf(out, length, "не удалось запустить Wire I2C");
      break;
    case Error::ClockLow:
      snprintf(out, length, "SCL=LOW (GPIO%u притянут к GND)", (unsigned)pinScl);
      break;
    case Error::DataLow:
      snprintf(out, length, "SDA=LOW (GPIO%u залип в нуле)", (unsigned)pinSda);
      break;
    case Error::WriteCommand:
      snprintf(out, length, "ошибка записи команды 0x%02X", (unsigned)diag.command);
      break;
    case Error::EndTransmission:
      snprintf(out, length, "нет ACK от 0x%02X (код Wire=%u, cmd=0x%02X)",
               (unsigned)addr, (unsigned)diag.wireCode, (unsigned)diag.command);
      break;
    case Error::ShortRead:
      snprintf(out, length, "неполное чтение (%u/%u байт от 0x%02X)",
               (unsigned)diag.received, (unsigned)diag.expected, (unsigned)addr);
      break;
    case Error::TemperatureCrc:
      snprintf(out, length, "ошибка CRC температуры: 0x%02X != 0x%02X",
               (unsigned)diag.crcReceived, (unsigned)diag.crcExpected);
      break;
    case Error::HumidityCrc:
      snprintf(out, length, "ошибка CRC влажности: 0x%02X != 0x%02X",
               (unsigned)diag.humCrcReceived, (unsigned)diag.humCrcExpected);
      break;
    case Error::ValueOutOfRange:
      snprintf(out, length, "значения T/RH вне допустимого диапазона");
      break;
    default:
      snprintf(out, length, "%s (код=%u, cmd=0x%02X)",
               errorName(), (unsigned)diag.wireCode, (unsigned)diag.command);
      break;
  }
}