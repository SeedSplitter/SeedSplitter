#include "qr_v1.h"
#include <string.h>

namespace {

static const char ALNUM[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
static const uint8_t RS_GEN_7[7] = {127, 122, 154, 164, 11, 68, 117};

static inline uint16_t qidx(uint8_t x, uint8_t y) {
  return (uint16_t)y * QR_V1_SIZE + x;
}

static inline void bit_set(uint8_t *bits, uint8_t x, uint8_t y, bool value) {
  const uint16_t i = qidx(x, y);
  const uint8_t mask = (uint8_t)(1U << (i & 7));
  if (value) bits[i >> 3] |= mask;
  else bits[i >> 3] &= (uint8_t)~mask;
}

static inline bool bit_get(const uint8_t *bits, uint8_t x, uint8_t y) {
  const uint16_t i = qidx(x, y);
  return ((bits[i >> 3] >> (i & 7)) & 1U) != 0;
}

static void function_module(uint8_t *qr, uint8_t *fn, int8_t x, int8_t y, bool dark) {
  if (x < 0 || y < 0 || x >= QR_V1_SIZE || y >= QR_V1_SIZE) return;
  bit_set(qr, (uint8_t)x, (uint8_t)y, dark);
  bit_set(fn, (uint8_t)x, (uint8_t)y, true);
}

static void draw_finder(uint8_t *qr, uint8_t *fn, int8_t cx, int8_t cy) {
  for (int8_t dy = -4; dy <= 4; ++dy) {
    for (int8_t dx = -4; dx <= 4; ++dx) {
      int8_t ax = dx < 0 ? -dx : dx;
      int8_t ay = dy < 0 ? -dy : dy;
      int8_t dist = ax > ay ? ax : ay;
      function_module(qr, fn, cx + dx, cy + dy, dist != 2 && dist != 4);
    }
  }
}

static inline bool int_bit(uint16_t value, uint8_t i) {
  return ((value >> i) & 1U) != 0;
}

static void draw_format_mask0(uint8_t *qr, uint8_t *fn) {
  // ECC L has formatBits=1. Mask pattern is fixed to 0.
  const uint8_t data = 1U << 3;
  uint16_t rem = data;
  for (uint8_t i = 0; i < 10; ++i)
    rem = (uint16_t)((rem << 1) ^ (((rem >> 9) & 1U) ? 0x537U : 0U));
  const uint16_t bits = (uint16_t)(((uint16_t)data << 10 | rem) ^ 0x5412U);

  for (uint8_t i = 0; i <= 5; ++i) function_module(qr, fn, 8, i, int_bit(bits, i));
  function_module(qr, fn, 8, 7, int_bit(bits, 6));
  function_module(qr, fn, 8, 8, int_bit(bits, 7));
  function_module(qr, fn, 7, 8, int_bit(bits, 8));
  for (uint8_t i = 9; i < 15; ++i) function_module(qr, fn, 14 - i, 8, int_bit(bits, i));

  for (uint8_t i = 0; i < 8; ++i)
    function_module(qr, fn, QR_V1_SIZE - 1 - i, 8, int_bit(bits, i));
  for (uint8_t i = 8; i < 15; ++i)
    function_module(qr, fn, 8, QR_V1_SIZE - 15 + i, int_bit(bits, i));

  // Fixed dark module.
  function_module(qr, fn, 8, QR_V1_SIZE - 8, true);
}

static void draw_functions(uint8_t *qr, uint8_t *fn) {
  for (uint8_t i = 0; i < QR_V1_SIZE; ++i) {
    function_module(qr, fn, 6, i, (i & 1U) == 0);
    function_module(qr, fn, i, 6, (i & 1U) == 0);
  }
  draw_finder(qr, fn, 3, 3);
  draw_finder(qr, fn, QR_V1_SIZE - 4, 3);
  draw_finder(qr, fn, 3, QR_V1_SIZE - 4);
  draw_format_mask0(qr, fn);
}

static int8_t alnum_value(char c) {
  for (uint8_t i = 0; i < 45; ++i) {
    if (ALNUM[i] == c) return (int8_t)i;
  }
  return -1;
}

static bool append_bits(uint8_t data[19], uint16_t &bit_len, uint16_t value, uint8_t count) {
  if ((uint16_t)(bit_len + count) > 152) return false;
  for (int8_t i = count - 1; i >= 0; --i) {
    if ((value >> i) & 1U)
      data[bit_len >> 3] |= (uint8_t)(1U << (7 - (bit_len & 7)));
    ++bit_len;
  }
  return true;
}

static bool make_data(const char *text, uint8_t data[19]) {
  size_t len = strlen(text);
  if (len > 25) return false;
  memset(data, 0, 19);

  uint16_t bit_len = 0;
  if (!append_bits(data, bit_len, 0x2, 4)) return false;       // alphanumeric mode
  if (!append_bits(data, bit_len, (uint16_t)len, 9)) return false;

  size_t pos = 0;
  while (pos + 1 < len) {
    int8_t a = alnum_value(text[pos]);
    int8_t b = alnum_value(text[pos + 1]);
    if (a < 0 || b < 0) return false;
    if (!append_bits(data, bit_len, (uint16_t)(45 * a + b), 11)) return false;
    pos += 2;
  }
  if (pos < len) {
    int8_t a = alnum_value(text[pos]);
    if (a < 0 || !append_bits(data, bit_len, (uint16_t)a, 6)) return false;
  }

  // Terminator (up to four zero bits), then byte alignment.
  uint8_t term = (uint16_t)(152 - bit_len) < 4 ? (uint8_t)(152 - bit_len) : 4;
  bit_len += term;  // buffer is already zeroed
  bit_len = (uint16_t)((bit_len + 7U) & ~7U);

  uint8_t used = (uint8_t)(bit_len >> 3);
  uint8_t pad = 0xEC;
  while (used < 19) {
    data[used++] = pad;
    pad = (pad == 0xEC) ? 0x11 : 0xEC;
  }
  return true;
}

static uint8_t gf_mul(uint8_t x, uint8_t y) {
  uint8_t z = 0;
  for (uint8_t i = 0; i < 8; ++i) {
    if (y & 1U) z ^= x;
    bool hi = (x & 0x80U) != 0;
    x <<= 1;
    if (hi) x ^= 0x1DU; // reduction by x^8+x^4+x^3+x^2+1 (0x11D)
    y >>= 1;
  }
  return z;
}

static void make_ecc(const uint8_t data[19], uint8_t ecc[7]) {
  memset(ecc, 0, 7);
  for (uint8_t i = 0; i < 19; ++i) {
    uint8_t factor = data[i] ^ ecc[0];
    for (uint8_t j = 0; j < 6; ++j)
      ecc[j] = ecc[j + 1] ^ gf_mul(RS_GEN_7[j], factor);
    ecc[6] = gf_mul(RS_GEN_7[6], factor);
  }
}

static inline bool codeword_bit(const uint8_t codewords[26], uint16_t i) {
  return ((codewords[i >> 3] >> (7 - (i & 7))) & 1U) != 0;
}

} // namespace

bool qr_v1_encode_alnum(const char *text, uint8_t out[QR_V1_PACKED_BYTES]) {
  uint8_t data[19];
  uint8_t ecc[7];
  uint8_t codewords[26];
  uint8_t fn[QR_V1_PACKED_BYTES];

  if (!text || !make_data(text, data)) return false;
  make_ecc(data, ecc);
  memcpy(codewords, data, 19);
  memcpy(codewords + 19, ecc, 7);

  memset(out, 0, QR_V1_PACKED_BYTES);
  memset(fn, 0, QR_V1_PACKED_BYTES);
  draw_functions(out, fn);

  uint16_t bit_index = 0;
  for (int8_t right = QR_V1_SIZE - 1; right >= 1; right -= 2) {
    if (right == 6) right = 5;
    for (uint8_t vert = 0; vert < QR_V1_SIZE; ++vert) {
      const bool upward = (((right + 1) & 2) == 0);
      const uint8_t y = upward ? (uint8_t)(QR_V1_SIZE - 1 - vert) : vert;
      for (uint8_t j = 0; j < 2; ++j) {
        const uint8_t x = (uint8_t)(right - j);
        if (!bit_get(fn, x, y) && bit_index < 208) {
          bool dark = codeword_bit(codewords, bit_index++);
          if (((x + y) & 1U) == 0) dark = !dark; // QR mask pattern 0
          bit_set(out, x, y, dark);
        }
      }
    }
  }
  return bit_index == 208;
}

bool qr_v1_get(const uint8_t qr[QR_V1_PACKED_BYTES], uint8_t x, uint8_t y) {
  if (x >= QR_V1_SIZE || y >= QR_V1_SIZE) return false;
  return bit_get(qr, x, y);
}
