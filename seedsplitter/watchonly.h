#ifndef SEEDSPLITTER_WATCHONLY_H
#define SEEDSPLITTER_WATCHONLY_H

#include <stddef.h>
#include <stdint.h>

#define WATCHONLY_ZPUB_MAX 116
#define WATCHONLY_BBQR_FRAME_MAX 25

// BIP39 (empty passphrase) -> BIP84 Bitcoin mainnet account 0 -> zpub.
// Derivation path: m/84'/0'/0'.
bool watchonly_make_zpub(const char *mnemonic, char *out, size_t out_size);

// BBQr Unicode/Base32 framing for a zpub.  Each non-final frame carries
// 16 Base32 chars so every Version-1/L QR stays <= 24 alphanumeric chars.
uint8_t watchonly_bbqr_frame_count(const char *zpub);
bool watchonly_bbqr_frame(const char *zpub, uint8_t frame_index,
                          char out[WATCHONLY_BBQR_FRAME_MAX]);

#endif
