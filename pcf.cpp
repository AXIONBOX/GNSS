// Minimal PCF8574 driver (quasi-bidirectional) - reused from AXION_BenchTest
#include <Wire.h>
#include "pins.h"
#include "pcf.h"

namespace {
  uint8_t shadow = 0xFF;
}

namespace pcf {
  static bool write_byte(uint8_t addr, uint8_t v){
    Wire.beginTransmission(addr);
    Wire.write(v);
    return Wire.endTransmission() == 0;
  }

  bool begin(uint8_t addr){
    shadow = 0xFF; // inputs high / outputs off
    return write_byte(addr, shadow);
  }

  void set_bit(uint8_t addr, uint8_t bit, bool high){
    if (high) shadow |= (uint8_t)(1u << bit);
    else      shadow &= (uint8_t)~(1u << bit);
    write_byte(addr, shadow);
  }

  bool read_byte(uint8_t addr, uint8_t& value){
    if (Wire.requestFrom((int)addr, 1) == 1){ value = (uint8_t)Wire.read(); return true; }
    return false;
  }

  bool read_bit(uint8_t addr, uint8_t bit){
    uint8_t v;
    if (!read_byte(addr, v)) return false;
    return ((v >> bit) & 1u) != 0;
  }
}

