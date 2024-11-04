// The _bit, _byte, and _word routines return the CRC of the len
// bytes at mem, applied to the previous CRC value, crc. If mem is
// NULL, then the other arguments are ignored, and the initial CRC,
// i.e. the CRC of zero bytes, is returned. Those routines will all
// return the same result, differing only in speed and code
// complexity. The _rem routine returns the CRC of the remaining
// bits in the last byte, for when the number of bits in the
// message is not a multiple of eight. The high bits bits of the low
// byte of val are applied to crc. bits must be in 0..8.

#include <stddef.h>
#include <stdint.h>

// Compute the CRC a bit at a time.
uint8_t crc5epc_c1g2_bit(uint8_t crc, void const *mem, size_t len);

// Compute the CRC of the high bits bits in the low byte of val.
uint8_t crc5epc_c1g2_rem(uint8_t crc, unsigned val, unsigned bits);

// Compute the CRC a byte at a time.
uint8_t crc5epc_c1g2_byte(uint8_t crc, void const *mem, size_t len);

// Compute the CRC a word at a time.
uint8_t crc5epc_c1g2_word(uint8_t crc, void const *mem, size_t len);

// Compute the combination of two CRCs.
uint8_t crc5epc_c1g2_comb(uint8_t crc1, uint8_t crc2, uintmax_t len2);
