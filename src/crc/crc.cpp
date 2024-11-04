#include <vector>

extern "C" {
#include "crc16genibus.h"
#include "crc5epc_c1g2.h"
}

std::vector<int> crc5(std::vector<int> bits) {
  int n_byte = bits.size() / 8;
  int n_rem = bits.size() % 8;
  unsigned char data[n_byte];
  for (int i = 0; i < n_byte; i++) {
    data[i] = 0;
    for (int j = 0; j < 8; j++) {
      data[i] |= bits[i * 8 + j] << (7 - j);
    }
  }
  uint8_t crc = n_byte == 0 ? 0x09 : crc5epc_c1g2_byte(0x9, data, n_byte);
  unsigned rem = 0;
  for (int i = 0; i < n_rem; i++) {
    rem |= bits[n_byte * 8 + i] << (7 - i);
  }
  crc = crc5epc_c1g2_rem(crc, rem, n_rem);
  std::vector<int> crc_bits(5);
  for (int i = 0; i < 5; i++) {
    crc_bits[4 - i] = crc >> i & 1;
  }
  return crc_bits;
}

std::vector<int> crc16(std::vector<int> bits) {
  int n_byte = bits.size() / 8;
  int n_rem = bits.size() % 8;
  unsigned char data[n_byte];
  for (int i = 0; i < n_byte; i++) {
    data[i] = 0;
    for (int j = 0; j < 8; j++) {
      data[i] |= bits[i * 8 + j] << (7 - j);
    }
  }
  uint16_t crc = n_byte == 0 ? 0x0000 : crc16genibus_byte(0x0000, data, n_byte);
  unsigned rem = 0;
  for (int i = 0; i < n_rem; i++) {
    rem |= bits[n_byte * 8 + i] << (7 - i);
  }
  crc = crc16genibus_rem(crc, rem, n_rem);
  std::vector<int> crc_bits(16);
  for (int i = 0; i < 16; i++) {
    crc_bits[15 - i] = crc >> i & 1;
  }
  return crc_bits;
}
