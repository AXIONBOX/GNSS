// Minimal PCF8574 driver (quasi-bidirectional) - reused from AXION_BenchTest
#pragma once
#include <Arduino.h>

namespace pcf {
  bool begin(uint8_t addr);
  void set_bit(uint8_t addr, uint8_t bit, bool high);
  bool read_byte(uint8_t addr, uint8_t& value);
  bool read_bit(uint8_t addr, uint8_t bit);
}

