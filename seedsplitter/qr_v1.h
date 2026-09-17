#ifndef SEEDSPLITTER_QR_V1_H
#define SEEDSPLITTER_QR_V1_H

#include <stdint.h>

// Minimal QR encoder for SeedSplitter's 128x32 OLED.
// Fixed format: QR Version 1, ECC level L, alphanumeric mode, mask 0.
// Capacity: up to 25 QR-alphanumeric characters.
#define QR_V1_SIZE 21
#define QR_V1_PACKED_BYTES 56

bool qr_v1_encode_alnum(const char *text, uint8_t out[QR_V1_PACKED_BYTES]);
bool qr_v1_get(const uint8_t qr[QR_V1_PACKED_BYTES], uint8_t x, uint8_t y);

#endif
