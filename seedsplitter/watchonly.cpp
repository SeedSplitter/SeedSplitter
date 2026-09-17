#include "watchonly.h"

#include <SHA256.h>
#include <SHA512.h>
#include <uECC.h>
#include <string.h>

namespace {

static const uint8_t SECP256K1_N[32] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,
  0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x41
};

static const char BASE58[] =
  "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static const char BASE32[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
static const char BASE36[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static void wipe(void *ptr, size_t n) {
  volatile uint8_t *p = (volatile uint8_t *)ptr;
  while (n--) *p++ = 0;
}

static void sha512_once(const void *data, size_t len, uint8_t out[64]) {
  SHA512 h;
  h.reset();
  h.update(data, len);
  h.finalize(out, 64);
  h.clear();
}

static void hmac_sha512(const uint8_t *key, size_t key_len,
                        const uint8_t *data, size_t data_len,
                        uint8_t out[64]) {
  SHA512 h;
  h.resetHMAC(key, key_len);
  h.update(data, data_len);
  h.finalizeHMAC(key, key_len, out, 64);
  h.clear();
}

// BIP39 PBKDF2-HMAC-SHA512 with empty passphrase.
// English BIP39 words are ASCII, so NFKD normalization is already satisfied.
static bool bip39_seed(const char *mnemonic, uint8_t seed[64]) {
  if (!mnemonic || !*mnemonic) return false;

  const size_t mnemonic_len = strlen(mnemonic);
  const uint8_t *key = (const uint8_t *)mnemonic;
  size_t key_len = mnemonic_len;
  uint8_t key_hash[64];

  // HMAC first hashes keys longer than SHA-512's 128-byte block.  Do that
  // once here so a 24-word mnemonic isn't re-hashed thousands of times.
  if (key_len > 128) {
    sha512_once(key, key_len, key_hash);
    key = key_hash;
    key_len = sizeof(key_hash);
  }

  static const uint8_t salt_block[] = {
    'm','n','e','m','o','n','i','c', 0x00,0x00,0x00,0x01
  };
  uint8_t u[64];

  hmac_sha512(key, key_len, salt_block, sizeof(salt_block), u);
  memcpy(seed, u, 64);
  for (uint16_t iter = 1; iter < 2048; ++iter) {
    hmac_sha512(key, key_len, u, sizeof(u), u);
    for (uint8_t i = 0; i < 64; ++i) seed[i] ^= u[i];
  }

  wipe(u, sizeof(u));
  wipe(key_hash, sizeof(key_hash));
  return true;
}

static int scalar_cmp(const uint8_t a[32], const uint8_t b[32]) {
  for (uint8_t i = 0; i < 32; ++i) {
    if (a[i] < b[i]) return -1;
    if (a[i] > b[i]) return 1;
  }
  return 0;
}

static bool scalar_zero(const uint8_t a[32]) {
  uint8_t v = 0;
  for (uint8_t i = 0; i < 32; ++i) v |= a[i];
  return v == 0;
}

// out = (a + b) mod n. Inputs are big-endian and strictly smaller than n.
static bool scalar_add_mod_n(const uint8_t a[32], const uint8_t b[32], uint8_t out[32]) {
  uint8_t sum[33];
  uint16_t carry = 0;
  sum[0] = 0;
  for (int8_t i = 31; i >= 0; --i) {
    uint16_t s = (uint16_t)a[i] + b[i] + carry;
    sum[i + 1] = (uint8_t)s;
    carry = s >> 8;
  }
  sum[0] = (uint8_t)carry;

  bool reduce = sum[0] != 0 || scalar_cmp(sum + 1, SECP256K1_N) >= 0;
  if (reduce) {
    int16_t borrow = 0;
    for (int8_t i = 32; i >= 0; --i) {
      const uint16_t nv = (i == 0) ? 0 : SECP256K1_N[i - 1];
      int16_t v = (int16_t)sum[i] - (int16_t)nv - borrow;
      if (v < 0) { v += 256; borrow = 1; }
      else borrow = 0;
      sum[i] = (uint8_t)v;
    }
  }

  memcpy(out, sum + 1, 32);
  wipe(sum, sizeof(sum));
  return !scalar_zero(out);
}

static void ser32(uint32_t v, uint8_t out[4]) {
  out[0] = (uint8_t)(v >> 24);
  out[1] = (uint8_t)(v >> 16);
  out[2] = (uint8_t)(v >> 8);
  out[3] = (uint8_t)v;
}

static bool compressed_pubkey(const uint8_t priv[32], uint8_t out[33]) {
  uint8_t raw[64];
  if (!uECC_compute_public_key(priv, raw, uECC_secp256k1())) {
    wipe(raw, sizeof(raw));
    return false;
  }
  out[0] = (raw[63] & 1U) ? 0x03 : 0x02;
  memcpy(out + 1, raw, 32);
  wipe(raw, sizeof(raw));
  return true;
}

static bool bip32_master(const uint8_t seed[64], uint8_t priv[32], uint8_t chain[32]) {
  static const uint8_t key[] = "Bitcoin seed";
  uint8_t i64[64];
  hmac_sha512(key, sizeof(key) - 1, seed, 64, i64);
  bool ok = !scalar_zero(i64) && scalar_cmp(i64, SECP256K1_N) < 0;
  if (ok) {
    memcpy(priv, i64, 32);
    memcpy(chain, i64 + 32, 32);
  }
  wipe(i64, sizeof(i64));
  return ok;
}

static bool bip32_hardened(uint8_t priv[32], uint8_t chain[32], uint32_t index) {
  uint8_t data[37];
  uint8_t i64[64];
  data[0] = 0;
  memcpy(data + 1, priv, 32);
  ser32(index | 0x80000000UL, data + 33);
  hmac_sha512(chain, 32, data, sizeof(data), i64);

  // BIP32: parse256(IL) >= n or resulting child == 0 is invalid.
  bool ok = scalar_cmp(i64, SECP256K1_N) < 0;
  if (ok) ok = scalar_add_mod_n(priv, i64, priv);
  if (ok) memcpy(chain, i64 + 32, 32);

  wipe(data, sizeof(data));
  wipe(i64, sizeof(i64));
  return ok;
}

static inline uint32_t rol32(uint32_t x, uint8_t n) {
  return (x << n) | (x >> (32 - n));
}

static uint32_t ripemd_f(uint8_t round, uint32_t x, uint32_t y, uint32_t z) {
  switch (round) {
    case 0: return x ^ y ^ z;
    case 1: return (x & y) | (~x & z);
    case 2: return (x | ~y) ^ z;
    case 3: return (x & z) | (y & ~z);
    default:return x ^ (y | ~z);
  }
}

static void ripemd160_32(const uint8_t msg[32], uint8_t out[20]) {
  static const uint8_t RL[80] = {
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
     7, 4,13, 1,10, 6,15, 3,12, 0, 9, 5, 2,14,11, 8,
     3,10,14, 4, 9,15, 8, 1, 2, 7, 0, 6,13,11, 5,12,
     1, 9,11,10, 0, 8,12, 4,13, 3, 7,15,14, 5, 6, 2,
     4, 0, 5, 9, 7,12, 2,10,14, 1, 3, 8,11, 6,15,13
  };
  static const uint8_t RR[80] = {
     5,14, 7, 0, 9, 2,11, 4,13, 6,15, 8, 1,10, 3,12,
     6,11, 3, 7, 0,13, 5,10,14,15, 8,12, 4, 9, 1, 2,
    15, 5, 1, 3, 7,14, 6, 9,11, 8,12, 2,10, 0, 4,13,
     8, 6, 4, 1, 3,11,15, 0, 5,12, 2,13, 9, 7,10,14,
    12,15,10, 4, 1, 5, 8, 7, 6, 2,13,14, 0, 3, 9,11
  };
  static const uint8_t SL[80] = {
    11,14,15,12, 5, 8, 7, 9,11,13,14,15, 6, 7, 9, 8,
     7, 6, 8,13,11, 9, 7,15, 7,12,15, 9,11, 7,13,12,
    11,13, 6, 7,14, 9,13,15,14, 8,13, 6, 5,12, 7, 5,
    11,12,14,15,14,15, 9, 8, 9,14, 5, 6, 8, 6, 5,12,
     9,15, 5,11, 6, 8,13,12, 5,12,13,14,11, 8, 5, 6
  };
  static const uint8_t SR[80] = {
     8, 9, 9,11,13,15,15, 5, 7, 7, 8,11,14,14,12, 6,
     9,13,15, 7,12, 8, 9,11, 7, 7,12, 7, 6,15,13,11,
     9, 7,15,11, 8, 6, 6,14,12,13, 5,14,13,13, 7, 5,
    15, 5, 8,11,14,14, 6,14, 6, 9,12, 9,12, 5,15, 8,
     8, 5,12, 9,12, 5,14, 6, 8,13, 6, 5,15,13,11,11
  };
  static const uint32_t KL[5] = {0x00000000UL,0x5A827999UL,0x6ED9EBA1UL,0x8F1BBCDCUL,0xA953FD4EUL};
  static const uint32_t KR[5] = {0x50A28BE6UL,0x5C4DD124UL,0x6D703EF3UL,0x7A6D76E9UL,0x00000000UL};

  uint8_t block[64];
  uint32_t x[16];
  memset(block, 0, sizeof(block));
  memcpy(block, msg, 32);
  block[32] = 0x80;
  block[57] = 0x01; // 32 bytes = 256 bits, little-endian length
  for (uint8_t i = 0; i < 16; ++i) {
    const uint8_t *p = block + 4 * i;
    x[i] = (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
  }

  uint32_t h0=0x67452301UL, h1=0xEFCDAB89UL, h2=0x98BADCFEUL,
           h3=0x10325476UL, h4=0xC3D2E1F0UL;
  uint32_t al=h0, bl=h1, cl=h2, dl=h3, el=h4;
  uint32_t ar=h0, br=h1, cr=h2, dr=h3, er=h4;

  for (uint8_t j = 0; j < 80; ++j) {
    uint8_t r = j >> 4;
    uint32_t tl = rol32(al + ripemd_f(r, bl, cl, dl) + x[RL[j]] + KL[r], SL[j]) + el;
    al=el; el=dl; dl=rol32(cl,10); cl=bl; bl=tl;

    uint32_t tr = rol32(ar + ripemd_f(4-r, br, cr, dr) + x[RR[j]] + KR[r], SR[j]) + er;
    ar=er; er=dr; dr=rol32(cr,10); cr=br; br=tr;
  }

  uint32_t t = h1 + cl + dr;
  h1 = h2 + dl + er;
  h2 = h3 + el + ar;
  h3 = h4 + al + br;
  h4 = h0 + bl + cr;
  h0 = t;
  uint32_t h[5] = {h0,h1,h2,h3,h4};
  for (uint8_t i = 0; i < 5; ++i) {
    out[4*i]   = (uint8_t)h[i];
    out[4*i+1] = (uint8_t)(h[i] >> 8);
    out[4*i+2] = (uint8_t)(h[i] >> 16);
    out[4*i+3] = (uint8_t)(h[i] >> 24);
  }

  wipe(block, sizeof(block));
  wipe(x, sizeof(x));
  wipe(h, sizeof(h));
}

static void hash160(const uint8_t pub[33], uint8_t out[20]) {
  uint8_t digest[32];
  SHA256 h;
  h.reset();
  h.update(pub, 33);
  h.finalize(digest, sizeof(digest));
  h.clear();
  ripemd160_32(digest, out);
  wipe(digest, sizeof(digest));
}

static void fingerprint(const uint8_t pub[33], uint8_t out[4]) {
  uint8_t h[20];
  hash160(pub, h);
  memcpy(out, h, 4);
  wipe(h, sizeof(h));
}

static bool base58check_zpub(uint8_t payload[82], char *out, size_t out_size) {
  // payload[0..77] is the extended key. Append the 4-byte double-SHA256 checksum.
  uint8_t digest[32];
  SHA256 h;
  h.reset(); h.update(payload, 78); h.finalize(digest, 32);
  h.reset(); h.update(digest, 32); h.finalize(digest, 32); h.clear();
  memcpy(payload + 78, digest, 4);
  wipe(digest, sizeof(digest));

  uint8_t digits[116];
  memset(digits, 0, sizeof(digits));
  size_t digits_len = 1;

  size_t zeroes = 0;
  while (zeroes < 82 && payload[zeroes] == 0) ++zeroes;
  for (size_t i = zeroes; i < 82; ++i) {
    uint16_t carry = payload[i];
    for (size_t j = 0; j < digits_len; ++j) {
      carry += (uint16_t)digits[j] << 8;
      digits[j] = (uint8_t)(carry % 58);
      carry /= 58;
    }
    while (carry) {
      if (digits_len >= sizeof(digits)) { wipe(digits, sizeof(digits)); return false; }
      digits[digits_len++] = (uint8_t)(carry % 58);
      carry /= 58;
    }
  }

  const size_t chars = zeroes + digits_len;
  if (chars + 1 > out_size) { wipe(digits, sizeof(digits)); return false; }
  size_t p = 0;
  for (size_t i = 0; i < zeroes; ++i) out[p++] = '1';
  for (size_t i = digits_len; i > 0; --i) out[p++] = BASE58[digits[i - 1]];
  out[p] = '\0';
  wipe(digits, sizeof(digits));
  return true;
}

static char b36(uint16_t v) { return BASE36[v % 36]; }

static size_t base32_length(size_t bytes) {
  return (bytes * 8U + 4U) / 5U;
}

static char base32_at(const uint8_t *data, size_t data_len, size_t char_index) {
  const size_t bit0 = char_index * 5U;
  uint8_t v = 0;
  for (uint8_t k = 0; k < 5; ++k) {
    const size_t bit = bit0 + k;
    v <<= 1;
    if (bit < data_len * 8U)
      v |= (data[bit >> 3] >> (7 - (bit & 7))) & 1U;
  }
  return BASE32[v];
}

} // namespace

bool watchonly_make_zpub(const char *mnemonic, char *out, size_t out_size) {
  if (!out || out_size == 0) return false;
  out[0] = '\0';

  uint8_t seed[64];
  uint8_t priv[32];
  uint8_t chain[32];
  uint8_t pub[33];
  uint8_t parent_fp[4];
  uint8_t serialized[82];
  bool ok = false;

  if (!bip39_seed(mnemonic, seed)) goto cleanup;
  if (!bip32_master(seed, priv, chain)) goto cleanup;

  // m / 84' / 0'
  if (!bip32_hardened(priv, chain, 84)) goto cleanup;
  if (!bip32_hardened(priv, chain, 0)) goto cleanup;

  // The account node's parent fingerprint belongs to m/84'/0'.
  if (!compressed_pubkey(priv, pub)) goto cleanup;
  fingerprint(pub, parent_fp);

  // m / 84' / 0' / 0'
  if (!bip32_hardened(priv, chain, 0)) goto cleanup;
  if (!compressed_pubkey(priv, pub)) goto cleanup;

  // SLIP-0132 zpub version = 0x04B24746, depth 3, child 0'.
  memset(serialized, 0, sizeof(serialized));
  serialized[0]=0x04; serialized[1]=0xB2; serialized[2]=0x47; serialized[3]=0x46;
  serialized[4]=3;
  memcpy(serialized + 5, parent_fp, 4);
  serialized[9]=0x80; serialized[10]=0; serialized[11]=0; serialized[12]=0;
  memcpy(serialized + 13, chain, 32);
  memcpy(serialized + 45, pub, 33);

  ok = base58check_zpub(serialized, out, out_size);

cleanup:
  wipe(seed, sizeof(seed));
  wipe(priv, sizeof(priv));
  wipe(chain, sizeof(chain));
  wipe(pub, sizeof(pub));
  wipe(parent_fp, sizeof(parent_fp));
  wipe(serialized, sizeof(serialized));
  if (!ok) out[0] = '\0';
  return ok;
}

uint8_t watchonly_bbqr_frame_count(const char *zpub) {
  if (!zpub || !*zpub) return 0;
  const size_t n = base32_length(strlen(zpub));
  const size_t frames = (n + 15U) / 16U;
  return frames > 255 ? 0 : (uint8_t)frames;
}

bool watchonly_bbqr_frame(const char *zpub, uint8_t frame_index,
                          char out[WATCHONLY_BBQR_FRAME_MAX]) {
  if (!zpub || !out) return false;
  const size_t zlen = strlen(zpub);
  const size_t b32len = base32_length(zlen);
  const uint8_t total = watchonly_bbqr_frame_count(zpub);
  if (!total || frame_index >= total || total >= 36U * 36U) return false;

  // BBQr header: B$ + Base32 encoding ('2') + Unicode text ('U')
  // + total parts (base36, two chars) + zero-based part index (two chars).
  out[0]='B'; out[1]='$'; out[2]='2'; out[3]='U';
  out[4]=b36(total / 36); out[5]=b36(total);
  out[6]=b36(frame_index / 36); out[7]=b36(frame_index);

  const size_t start = (size_t)frame_index * 16U;
  size_t count = b32len - start;
  if (count > 16) count = 16;
  for (size_t i = 0; i < count; ++i)
    out[8 + i] = base32_at((const uint8_t *)zpub, zlen, start + i);
  out[8 + count] = '\0';
  return true;
}
