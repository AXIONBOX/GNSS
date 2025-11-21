#include <Arduino.h>
#include <SPI.h>

#include "hardware/display_ssd1322.h"
#include "system/pins.h"
#include "system/pcf.h"

namespace {

  static SPIClass* s_vspi = nullptr;

  static inline void oled_dc_cmd()  { pcf::set_bit(I2C_ADDR_PCF8574, PCF_BIT_OLED_DC, false); }
  static inline void oled_dc_data() { pcf::set_bit(I2C_ADDR_PCF8574, PCF_BIT_OLED_DC, true);  }
  static inline void oled_reset_assert()  { pcf::set_bit(I2C_ADDR_PCF8574, PCF_BIT_OLED_RST, false); }
  static inline void oled_reset_release() { pcf::set_bit(I2C_ADDR_PCF8574, PCF_BIT_OLED_RST, true);  }

  static inline void ssd1322_send_cmd(uint8_t cmd)
  {
    s_vspi->beginTransaction(SPISettings(12000000, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_OLED_CS, LOW);
    oled_dc_cmd();
    s_vspi->transfer(cmd);
    digitalWrite(PIN_OLED_CS, HIGH);
    s_vspi->endTransaction();
  }

  static inline void ssd1322_send_data(uint8_t dat)
  {
    s_vspi->beginTransaction(SPISettings(12000000, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_OLED_CS, LOW);
    oled_dc_data();
    s_vspi->transfer(dat);
    digitalWrite(PIN_OLED_CS, HIGH);
    s_vspi->endTransaction();
  }

  static void ssd1322_init()
  {
    digitalWrite(PIN_OLED_CS, HIGH);

    oled_reset_assert();
    delay(100);
    oled_reset_release();
    delay(100);

    oled_dc_cmd();
    digitalWrite(PIN_OLED_CS, HIGH);

    ssd1322_send_cmd(0xFD); ssd1322_send_data(0x12);
    ssd1322_send_cmd(0xAE);
    ssd1322_send_cmd(0xB3); ssd1322_send_data(0x91);
    ssd1322_send_cmd(0xCA); ssd1322_send_data(0x3F);
    ssd1322_send_cmd(0xA2); ssd1322_send_data(0x00);
    ssd1322_send_cmd(0xA1); ssd1322_send_data(0x00);
    ssd1322_send_cmd(0xA0); ssd1322_send_data(0x14); ssd1322_send_data(0x11);

    ssd1322_send_cmd(0xB3); ssd1322_send_data(0xF1);
    ssd1322_send_cmd(0xB4); ssd1322_send_data(0xA0); ssd1322_send_data(0xFF);
    ssd1322_send_cmd(0xB9);
    ssd1322_send_cmd(0xC1); ssd1322_send_data(0xFF);
    ssd1322_send_cmd(0xC7); ssd1322_send_data(0x0F);
    ssd1322_send_cmd(0xBE); ssd1322_send_data(0x07);

    ssd1322_send_cmd(0xB8);
    for (int i = 0; i < 16; ++i) {
      int v = i * 16;
      if (v > 255) v = 255;
      ssd1322_send_data((uint8_t)v);
    }

    ssd1322_send_cmd(0xAB); ssd1322_send_data(0x01);
    ssd1322_send_cmd(0xB1); ssd1322_send_data(0xE2);
    ssd1322_send_cmd(0xBB); ssd1322_send_data(0x1F);
    ssd1322_send_cmd(0xA6);
    ssd1322_send_cmd(0xAF);
  }

} // namespace

void display_init()
{
  if (s_vspi == nullptr) {
    pinMode(PIN_OLED_SCLK, OUTPUT);
    pinMode(PIN_OLED_MOSI, OUTPUT);
    pinMode(PIN_OLED_CS,   OUTPUT);
    digitalWrite(PIN_OLED_CS, HIGH);

    s_vspi = new SPIClass(HSPI);
    s_vspi->begin(PIN_OLED_SCLK, -1, PIN_OLED_MOSI, -1);
    s_vspi->setFrequency(20000000);
    s_vspi->setDataMode(SPI_MODE0);
    s_vspi->setBitOrder(MSBFIRST);
  }

  ssd1322_init();
}

void display_full_reinit()
{
  Serial.println("Full OLED re-init (SPI+PCF+SSD1322)...");

  if (s_vspi) {
    s_vspi->end();
    delete s_vspi;
    s_vspi = nullptr;
  }

  pcf::begin(I2C_ADDR_PCF8574);

  display_init();
}

void display_push(uint8_t* buf, int w, int h)
{
  if (!s_vspi) {
    // Si l'écran n'est pas initialisé, on évite de planter.
    display_init();
    if (!s_vspi) return;
  }

  ssd1322_send_cmd(0x15); ssd1322_send_data(0x1C); ssd1322_send_data(0x5B);
  ssd1322_send_cmd(0x75); ssd1322_send_data(0x00); ssd1322_send_data(0x3F);
  ssd1322_send_cmd(0x5C);
  delayMicroseconds(2);

  s_vspi->beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  oled_dc_data();
  digitalWrite(PIN_OLED_CS, LOW);

  uint8_t line[128];
  for (int y = 0; y < h; ++y) {
    int row = y * w;
    for (int x = 0, b = 0; x < w; x += 2, ++b) {
      uint8_t g1 = buf[row + x] >> 4;
      uint8_t g2 = buf[row + x + 1] >> 4;
      line[b] = (uint8_t)((g1 << 4) | g2);
    }
    s_vspi->transferBytes(line, nullptr, 128);
  }

  digitalWrite(PIN_OLED_CS, HIGH);
  s_vspi->endTransaction();
}

