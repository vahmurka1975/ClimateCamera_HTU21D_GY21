#include "Sht30Reliable.h"
#include <math.h>
#include <stdio.h>

// ===================== ПРЯМОЙ ПРОГРАММНЫЙ I2C ДРАЙВЕР HTU21D (GY-21) =====================
// Пины: SDA = GPIO 21, SCL = GPIO 22, адрес HTU21D / SHT21 (GY-21) = 0x40
// Работает через открытый сток (Open-Drain) с внутренней подтяжкой.
#define HTU21_SDA_PIN 21
#define HTU21_SCL_PIN 22
#define HTU21_ADDR    0x40
#define HTU21_HALF_PERIOD_US 20  // ~25 кГц — надёжный запас фронта сигнала для кабеля 150 см

static constexpr uint8_t CMD_TRIG_TEMP_NOHOLD = 0xF3;
static constexpr uint8_t CMD_TRIG_HUM_NOHOLD  = 0xF5;
static constexpr uint8_t CMD_WRITE_USER_REG   = 0xE6;
static constexpr uint8_t CMD_READ_USER_REG    = 0xE7;
static constexpr uint8_t CMD_SOFT_RESET       = 0xFE;

static inline void sht30SdaHigh() { pinMode(HTU21_SDA_PIN, INPUT_PULLUP); }
static inline void sht30SdaLow()  { digitalWrite(HTU21_SDA_PIN, LOW); pinMode(HTU21_SDA_PIN, OUTPUT); }

static inline void sht30SclHigh() {
  pinMode(HTU21_SCL_PIN, INPUT_PULLUP);
  uint32_t t0 = micros();
  while (digitalRead(HTU21_SCL_PIN) == LOW && (uint32_t)(micros() - t0) < 2000UL) {
    // Ожидание отпускания линии SCL (Clock Stretching)
  }
}
static inline void sht30SclLow()  { digitalWrite(HTU21_SCL_PIN, LOW); pinMode(HTU21_SCL_PIN, OUTPUT); }

static void sht30Start() {
  sht30SdaHigh(); sht30SclHigh(); delayMicroseconds(HTU21_HALF_PERIOD_US);
  sht30SdaLow();  delayMicroseconds(HTU21_HALF_PERIOD_US);
  sht30SclLow();  delayMicroseconds(HTU21_HALF_PERIOD_US);
}

static void sht30Stop() {
  sht30SdaLow();  delayMicroseconds(HTU21_HALF_PERIOD_US);
  sht30SclHigh(); delayMicroseconds(HTU21_HALF_PERIOD_US);
  sht30SdaHigh(); delayMicroseconds(HTU21_HALF_PERIOD_US);
}

static bool sht30WriteByte(uint8_t data) {
  for (uint8_t i = 0; i < 8; ++i) {
    if (data & 0x80) sht30SdaHigh(); else sht30SdaLow();
    data <<= 1;
    delayMicroseconds(HTU21_HALF_PERIOD_US);
    sht30SclHigh();
    delayMicroseconds(HTU21_HALF_PERIOD_US);
    sht30SclLow();
  }
  sht30SdaHigh();
  delayMicroseconds(HTU21_HALF_PERIOD_US);
  sht30SclHigh();
  delayMicroseconds(HTU21_HALF_PERIOD_US);
  bool ack = (digitalRead(HTU21_SDA_PIN) == LOW);
  sht30SclLow();
  return ack;
}

static uint8_t sht30ReadByte(bool sendAck) {
  uint8_t data = 0;
  sht30SdaHigh();
  for (uint8_t i = 0; i < 8; ++i) {
    data <<= 1;
    delayMicroseconds(HTU21_HALF_PERIOD_US);
    sht30SclHigh();
    delayMicroseconds(HTU21_HALF_PERIOD_US);
    if (digitalRead(HTU21_SDA_PIN)) data |= 1;
    sht30SclLow();
  }
  if (sendAck) sht30SdaLow(); else sht30SdaHigh();
  delayMicroseconds(HTU21_HALF_PERIOD_US);
  sht30SclHigh();
  delayMicroseconds(HTU21_HALF_PERIOD_US);
  sht30SclLow();  sht30SdaHigh();
  delayMicroseconds(HTU21_HALF_PERIOD_US);
  return data;
}

// CRC-8 для HTU21D / SHT21 (GY-21): полином 0x31 (x^8 + x^5 + x^4 + 1), начальное значение 0x00
uint8_t sht30Crc8(const uint8_t *data, size_t len) {
  uint8_t crc = 0x00;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; ++b) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}

bool sht30SendCmd(uint8_t cmd) {
  sht30Start();
  if (!sht30WriteByte((HTU21_ADDR << 1) | 0)) { sht30Stop(); return false; }
  if (!sht30WriteByte(cmd))                   { sht30Stop(); return false; }
  sht30Stop();
  return true;
}

void initSHT30() {
  sht30SdaHigh();
  sht30SclHigh();
  delay(2);
  // 9 тактов SCL при SDA=HIGH + STOP + Soft Reset (0xFE)
  for (int i = 0; i < 9; ++i) {
    sht30SclLow();  delayMicroseconds(HTU21_HALF_PERIOD_US);
    sht30SclHigh(); delayMicroseconds(HTU21_HALF_PERIOD_US);
  }
  sht30Start();
  sht30Stop();
  delay(5);
  sht30SendCmd(CMD_SOFT_RESET);
  delay(50);
}

static bool htu21Read3(uint8_t b[3]) {
  bool gotAck = false;
  for (int attempt = 0; attempt < 25; ++attempt) {
    sht30Start();
    if (sht30WriteByte((HTU21_ADDR << 1) | 1)) {
      gotAck = true;
      break;
    }
    sht30Stop();
    delay(3);
  }
  if (!gotAck) return false;
  for (int i = 0; i < 3; ++i) {
    b[i] = sht30ReadByte(i < 2);
  }
  sht30Stop();
  return true;
}

bool readSHT30(float &outTemp, float &outHum) {
  if (!sht30SendCmd(CMD_TRIG_TEMP_NOHOLD)) {
    initSHT30();
    if (!sht30SendCmd(CMD_TRIG_TEMP_NOHOLD)) return false;
  }
  delay(85); // Ожидание конверсии температуры 14 бит (HTU21D / SHT21 GY-21)
  uint8_t tb[3] = {0};
  if (!htu21Read3(tb) || sht30Crc8(tb, 2) != tb[2]) {
    initSHT30();
    return false;
  }

  if (!sht30SendCmd(CMD_TRIG_HUM_NOHOLD)) return false;
  delay(50); // Ожидание конверсии влажности 12 бит (HTU21D / SHT21 GY-21)
  uint8_t hb[3] = {0};
  if (!htu21Read3(hb) || sht30Crc8(hb, 2) != hb[2]) {
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
// ================================================================================

Sht30Reliable::Sht30Reliable(TwoWire &wire,uint8_t sda,uint8_t scl,uint8_t address)
  : bus(wire),pinSda(sda),pinScl(scl),addr(address) {}

void Sht30Reliable::sdaHigh() const { pinMode(pinSda, INPUT_PULLUP); }
void Sht30Reliable::sdaLow() const  { digitalWrite(pinSda, LOW); pinMode(pinSda, OUTPUT); }
void Sht30Reliable::sclHigh() const {
  pinMode(pinScl, INPUT_PULLUP);
  uint32_t t0 = micros();
  while (digitalRead(pinScl) == LOW && (uint32_t)(micros() - t0) < 2000UL) {}
}
void Sht30Reliable::sclLow() const  { digitalWrite(pinScl, LOW); pinMode(pinScl, OUTPUT); }

void Sht30Reliable::i2cStart() const {
  sdaHigh(); sclHigh(); delayMicroseconds(HTU21_HALF_PERIOD_US);
  sdaLow();  delayMicroseconds(HTU21_HALF_PERIOD_US);
  sclLow();  delayMicroseconds(HTU21_HALF_PERIOD_US);
}

void Sht30Reliable::i2cStop() const {
  sdaLow();  delayMicroseconds(HTU21_HALF_PERIOD_US);
  sclHigh(); delayMicroseconds(HTU21_HALF_PERIOD_US);
  sdaHigh(); delayMicroseconds(HTU21_HALF_PERIOD_US);
}

bool Sht30Reliable::i2cWriteByte(uint8_t data) const {
  for (uint8_t i = 0; i < 8; ++i) {
    if (data & 0x80) sdaHigh(); else sdaLow();
    data <<= 1;
    delayMicroseconds(HTU21_HALF_PERIOD_US);
    sclHigh();
    delayMicroseconds(HTU21_HALF_PERIOD_US);
    sclLow();
  }
  sdaHigh();
  delayMicroseconds(HTU21_HALF_PERIOD_US);
  sclHigh();
  delayMicroseconds(HTU21_HALF_PERIOD_US);
  bool ack = (digitalRead(pinSda) == LOW);
  sclLow();
  return ack;
}

uint8_t Sht30Reliable::i2cReadByte(bool ack) const {
  uint8_t data = 0;
  sdaHigh();
  for (uint8_t i = 0; i < 8; ++i) {
    data <<= 1;
    delayMicroseconds(HTU21_HALF_PERIOD_US);
    sclHigh();
    delayMicroseconds(HTU21_HALF_PERIOD_US);
    if (digitalRead(pinSda)) data |= 1;
    sclLow();
  }
  if (ack) sdaLow(); else sdaHigh();
  delayMicroseconds(HTU21_HALF_PERIOD_US);
  sclHigh();
  delayMicroseconds(HTU21_HALF_PERIOD_US);
  sclLow();
  sdaHigh();
  delayMicroseconds(HTU21_HALF_PERIOD_US);
  return data;
}

uint8_t Sht30Reliable::crc8(const uint8_t *data,size_t length) {
  return sht30Crc8(data, length);
}

const char *Sht30Reliable::errorName() const {
  switch(diag.error) {
    case Error::None: return "none";
    case Error::I2cBegin: return "i2c_begin";
    case Error::ClockLow: return "scl_stuck_low";
    case Error::DataLow: return "sda_stuck_low";
    case Error::WriteCommand: return "command_write";
    case Error::EndTransmission: return "end_transmission";
    case Error::ShortRead: return "short_read";
    case Error::TemperatureCrc: return "temperature_crc";
    case Error::HumidityCrc: return "humidity_crc";
    case Error::BothCrc: return "temperature_and_humidity_crc";
    case Error::StatusCrc: return "status_crc";
    case Error::StatusFlags: return "status_flags";
    case Error::ValueOutOfRange: return "value_out_of_range";
  }
  return "unknown";
}

void Sht30Reliable::formatError(char *out,size_t length) const {
  if(!out || !length) return;
  switch(diag.error) {
    case Error::None: snprintf(out,length,"нет ошибки"); break;
    case Error::I2cBegin: snprintf(out,length,"не удалось запустить I2C"); break;
    case Error::ClockLow: snprintf(out,length,"SCL удерживается в LOW"); break;
    case Error::DataLow: snprintf(out,length,"SDA удерживается в LOW"); break;
    case Error::WriteCommand:
      snprintf(out,length,"ошибка записи команды 0x%02X",diag.command); break;
    case Error::EndTransmission:
      snprintf(out,length,"нет ACK от датчика (код %u, команда 0x%02X)",diag.wireCode,diag.command); break;
    case Error::ShortRead:
      snprintf(out,length,"нет ответа при чтении (%u/%u байт)",diag.received,diag.expected); break;
    case Error::TemperatureCrc:
      snprintf(out,length,"неверная CRC температуры: 0x%02X, ожидалась 0x%02X",diag.crcReceived,diag.crcExpected); break;
    case Error::HumidityCrc:
      snprintf(out,length,"неверная CRC влажности: 0x%02X, ожидалась 0x%02X",diag.humCrcReceived,diag.humCrcExpected); break;
    case Error::BothCrc:
      snprintf(out,length,"неверная CRC T 0x%02X/0x%02X и RH 0x%02X/0x%02X",
               diag.crcReceived,diag.crcExpected,diag.humCrcReceived,diag.humCrcExpected); break;
    case Error::StatusCrc:
      snprintf(out,length,"неверная CRC статуса: 0x%02X, ожидалась 0x%02X",diag.crcReceived,diag.crcExpected); break;
    case Error::StatusFlags:
      snprintf(out,length,"ошибка регистра HTU21D: 0x%02X",diag.status); break;
    case Error::ValueOutOfRange:
      snprintf(out,length,"измерения вне допустимого диапазона"); break;
  }
}

bool Sht30Reliable::command(uint8_t value) {
  diag.command=value;
  i2cStart();
  if(!i2cWriteByte((uint8_t)((addr<<1)|0))) {
    i2cStop();
    diag.error=Error::EndTransmission;
    diag.wireCode=2; // NACK on address
    return false;
  }
  if(!i2cWriteByte(value)) {
    i2cStop();
    diag.error=Error::WriteCommand;
    diag.wireCode=3; // NACK on command byte
    return false;
  }
  i2cStop();
  return true;
}

bool Sht30Reliable::readBytes(uint8_t *data,size_t length) {
  diag.expected=(uint8_t)length;
  diag.received=0;
  bool gotAck=false;
  for(uint8_t attempt=0;attempt<25;++attempt) {
    i2cStart();
    if(i2cWriteByte((uint8_t)((addr<<1)|1))) {
      gotAck=true;
      break;
    }
    i2cStop();
    delay(3);
  }
  if(!gotAck) {
    diag.error=Error::ShortRead;
    return false;
  }
  for(size_t i=0;i<length;++i) {
    data[i]=i2cReadByte(i+1<length);
    diag.received=(uint8_t)(i+1);
  }
  i2cStop();
  return true;
}

bool Sht30Reliable::waitClockHigh() {
  uint32_t start=micros();
  while(digitalRead(pinScl)==LOW) {
    if((uint32_t)(micros()-start)>=2000UL) return false;
    delayMicroseconds(5);
  }
  return true;
}

bool Sht30Reliable::begin() { return recover(); }

bool Sht30Reliable::recover() {
  diag=Diagnostic();
  // Освобождаем аппаратный драйвер Wire, если он был активен, и переводим пины в Open-Drain
  bus.end();
  sdaHigh();
  sclHigh();
  delay(2);
  if(!waitClockHigh()) {
    diag.error=Error::ClockLow;
    return false;
  }
  // 9 тактов SCL при SDA=HIGH + STOP для гарантированного освобождения шины I2C
  for(uint8_t i=0;i<9;++i) {
    sclLow();  delayMicroseconds(HTU21_HALF_PERIOD_US);
    sclHigh();
    if(!waitClockHigh()) { diag.error=Error::ClockLow; return false; }
    delayMicroseconds(HTU21_HALF_PERIOD_US);
  }
  i2cStart();
  i2cStop();
  delay(5);
  if(digitalRead(pinSda)==LOW) {
    diag.error=Error::DataLow;
    return false;
  }
  // Программный сброс HTU21D / SHT21 (0xFE) и ожидание перезапуска (50 мс для всех ревизий GY-21)
  if(!command(CMD_SOFT_RESET)) return false;
  delay(50);
  diag=Diagnostic();
  return true;
}

bool Sht30Reliable::startMeasurement() {
  diag=Diagnostic();
  if(!command(CMD_TRIG_TEMP_NOHOLD)) {
    recover();
    if(!command(CMD_TRIG_TEMP_NOHOLD)) return false;
  }
  return true;
}

bool Sht30Reliable::readMeasurement(float &temperature,float &humidity) {
  diag=Diagnostic();
  // 1. Читаем результат конверсии температуры (3 байта: MSB, LSB, CRC8)
  uint8_t tBytes[3]={0};
  if(!readBytes(tBytes,sizeof(tBytes))) return false;
  diag.crcExpected=crc8(tBytes,2);
  diag.crcReceived=tBytes[2];
  if(diag.crcExpected!=diag.crcReceived) {
    diag.error=Error::TemperatureCrc;
    return false;
  }
  uint16_t rawT=(((uint16_t)tBytes[0]<<8)|tBytes[1]) & 0xFFFC;

  // 2. Запускаем измерение влажности No-Hold Master (0xF5) и ждём конверсию 12 бит (50 мс для HTU21D / SHT21)
  if(!command(CMD_TRIG_HUM_NOHOLD)) return false;
  delay(50);
  uint8_t hBytes[3]={0};
  if(!readBytes(hBytes,sizeof(hBytes))) return false;
  diag.humCrcExpected=crc8(hBytes,2);
  diag.humCrcReceived=hBytes[2];
  if(diag.humCrcExpected!=diag.humCrcReceived) {
    diag.error=Error::HumidityCrc;
    return false;
  }
  uint16_t rawH=(((uint16_t)hBytes[0]<<8)|hBytes[1]) & 0xFFFC;

  // 3. Пересчёт по формулам даташита HTU21D / SHT21
  float t=-46.85f+175.72f*((float)rawT/65536.0f);
  float h=-6.0f+125.0f*((float)rawH/65536.0f);
  if(h<0.0f && h>=-6.0f) h=0.0f;
  if(h>100.0f && h<=119.0f) h=100.0f;

  if(!isfinite(t) || !isfinite(h) || t<-40.0f || t>125.0f || h<0.0f || h>100.0f) {
    diag.error=Error::ValueOutOfRange;
    return false;
  }
  temperature=t;
  humidity=h;
  return true;
}
