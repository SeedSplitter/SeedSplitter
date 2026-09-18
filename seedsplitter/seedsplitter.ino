#include <Tiny4kOLED.h>
#include <SHA256.h>
#include "bip39_words_packed.h"
#include "watchonly.h"
#include "qr_v1.h"

// Menus / modes
#define MAIN_MENU       0
#define WORD_INPUT      1
#define SHOW_SEED       2
#define SHOW_SHARES     3
#define SEED_LENGTH     4
#define DICE_INPUT      5
#define INFO_SCREEN     6
#define SHOW_ZPUB_QR    7
#define INVALID_SEED    8
#define ZPUB_ERROR     9

#define MODE_SPLIT      0
#define MODE_RECOVER    1
#define MODE_NEW_SEED   2
#define MODE_ZPUB    3

#define BOTON_RIGHT PA1
#define BOTON_LEFT PA2
#define BOTON_ENTER PA0
#define PRIMER_RENGLON 0
#define SEGUNDO_RENGLON 10
#define LARGO_LETRA 8

#define ENTER 4
#define LEFT 2
#define RIGHT 1
#define LETRA_BORRA 27
#define LETRA_ESPACIO 26

static constexpr uint8_t DICE_ROLLS = 50;
static constexpr uint16_t QR_FRAME_MS = 300;
static constexpr uint8_t QR_BITMAP_SIDE = 29;
static constexpr uint8_t QR_X = 0;
static constexpr uint8_t QR_TEXT_X = 40;

SHA256 sha256;

static uint8_t menu = MAIN_MENU;
static uint8_t palabra[4] = { 0xff, 0xff, 0xff, 0xff };
static uint8_t op_cont = 0;
static uint8_t word_cont = 0;
static uint8_t letter_cont = 0;
// Debounced button state. The 16-bit timer is enough because the
// debounce interval is only a few milliseconds; wrap-around is harmless.
static uint8_t raw_buttons_last = 0;
static uint8_t stable_buttons = 0;
static uint16_t debounce_since = 0;

// BIP39 indexes are 11-bit values, so uint16_t is sufficient.
static uint16_t seed_phrase[24];

// Split/Recover and New Seed never need these buffers at the same time.
union Workspace {
  uint16_t share[3][24];
  uint8_t dice_rolls[DICE_ROLLS];
};
static Workspace workspace;

static uint8_t seed_length = 12;
static uint8_t seleccion = MODE_SPLIT;
static uint8_t seeds_cargadas = 0;

static uint8_t dice_count = 0;
static uint8_t dice_value = 1; // 1..6, 7 = back

// Public watch-only export.  Private seed/key material is only temporary.
static char watch_zpub[WATCHONLY_ZPUB_MAX];
static bool watch_zpub_ready = false;
static uint8_t qr_frame_index = 0;
static uint16_t qr_last_change = 0;

void split();
void recover();
static void show_main_menu();
static inline void pad_line();

static void secure_zero(void *ptr, size_t n) {
  volatile uint8_t *p = (volatile uint8_t *)ptr;
  while (n--) *p++ = 0;
}

// Decode one character from the 5-byte packed BIP39 table.
// Returns '\0' after the end of a word.
static char word_char(uint16_t index, uint8_t pos) {
  if (pos >= 8) return '\0';
  const uint8_t bit = pos * 5;
  const uint8_t b = bit >> 3;
  const uint8_t shift = bit & 7;
  uint16_t x = BIP39_WORDS[index][b];
  if (b < 4) x |= ((uint16_t)BIP39_WORDS[index][b + 1]) << 8;
  const uint8_t code = (x >> shift) & 0x1f;
  return code ? (char)('a' + code - 1) : '\0';
}

static void print_word(uint16_t index) {
  for (uint8_t i = 0; i < 8; i++) {
    const char c = word_char(index, i);
    if (!c) break;
    oled.write(c);
  }
}

// Reconstruct the exact English BIP39 mnemonic text from the indexes already
// stored in seed_phrase.  24 * 8 letters + 23 spaces + NUL = 216 bytes max.
static bool build_mnemonic(char *out, size_t out_size) {
  size_t pos = 0;
  for (uint8_t w = 0; w < seed_length; ++w) {
    if (w) {
      if (pos + 1 >= out_size) return false;
      out[pos++] = ' ';
    }
    for (uint8_t i = 0; i < 8; ++i) {
      const char c = word_char(seed_phrase[w], i);
      if (!c) break;
      if (pos + 1 >= out_size) return false;
      out[pos++] = c;
    }
  }
  if (pos >= out_size) return false;
  out[pos] = '\0';
  return true;
}

static bool generate_watchonly_zpub() {
  char mnemonic[216];
  watch_zpub_ready = false;
  watch_zpub[0] = '\0';
  if (!build_mnemonic(mnemonic, sizeof(mnemonic))) {
    secure_zero(mnemonic, sizeof(mnemonic));
    return false;
  }
  const bool ok = watchonly_make_zpub(mnemonic, watch_zpub, sizeof(watch_zpub));
  secure_zero(mnemonic, sizeof(mnemonic));
  watch_zpub_ready = ok;
  return ok;
}

// Convert a 21x21 QR into a 29x32 OLED bitmap.  The 4-pixel quiet zone is
// included.  OLED lit pixels are the white QR background; dark QR modules are
// left unlit, matching the normal black-on-white QR polarity.
static void qr_to_oled_bitmap(const uint8_t qr[QR_V1_PACKED_BYTES], uint8_t bitmap[QR_BITMAP_SIDE * 4]) {
  memset(bitmap, 0xFF, QR_BITMAP_SIDE * 4);
  for (uint8_t y = 0; y < QR_V1_SIZE; ++y) {
    for (uint8_t x = 0; x < QR_V1_SIZE; ++x) {
      if (!qr_v1_get(qr, x, y)) continue;
      const uint8_t px = x + 4;
      const uint8_t py = y + 4;
      bitmap[(py >> 3) * QR_BITMAP_SIDE + px] &= (uint8_t)~(1U << (py & 7));
    }
  }
}

static void draw_watchonly_qr_frame() {
  if (!watch_zpub_ready) return;

  char frame[WATCHONLY_BBQR_FRAME_MAX];
  uint8_t qr[QR_V1_PACKED_BYTES];
  uint8_t bitmap[QR_BITMAP_SIDE * 4];
  const uint8_t total = watchonly_bbqr_frame_count(watch_zpub);
  if (!total) return;
  if (qr_frame_index >= total) qr_frame_index = 0;

  if (watchonly_bbqr_frame(watch_zpub, qr_frame_index, frame) &&
      qr_v1_encode_alnum(frame, qr)) {
    qr_to_oled_bitmap(qr, bitmap);
    oled.bitmap(QR_X, 0, QR_X + QR_BITMAP_SIDE, 4, bitmap);

    // Keep the public export visually separate from the seed words.
    // The 29x29 QR sits at the far left; the two available text rows
    // identify the purpose and show progress through the animated BBQr.
    oled.setCursor(QR_TEXT_X, PRIMER_RENGLON);
    oled.print("Watch-only");
    pad_line();

    oled.setCursor(QR_TEXT_X, SEGUNDO_RENGLON);
    oled.print("zpub ");
    if (qr_frame_index + 1 < 10) oled.write('0');
    oled.print(qr_frame_index + 1);
    oled.write('/');
    if (total < 10) oled.write('0');
    oled.print(total);
    pad_line();
    oled.on();
  }

  secure_zero(frame, sizeof(frame));
  secure_zero(qr, sizeof(qr));
  secure_zero(bitmap, sizeof(bitmap));
}

static void enter_watchonly_qr() {
  if (!watch_zpub_ready) return;
  oled.clear();
  qr_frame_index = 0;
  qr_last_change = (uint16_t)millis();
  draw_watchonly_qr_frame();
  menu = SHOW_ZPUB_QR;
}

static void update_watchonly_qr() {
  const uint16_t now = (uint16_t)millis();
  if ((uint16_t)(now - qr_last_change) < QR_FRAME_MS) return;
  qr_last_change = now;
  const uint8_t total = watchonly_bbqr_frame_count(watch_zpub);
  if (!total) return;
  qr_frame_index = (uint8_t)((qr_frame_index + 1) % total);
  draw_watchonly_qr_frame();
}

// Lexicographic comparison between a word and a prefix encoded as 0..25,
// using LETRA_ESPACIO at position 4 to mean the end of a 3-letter word.
static int8_t compare_word_prefix(uint16_t index, const uint8_t *prefix, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    const char wc = word_char(index, i);
    const char pc = (prefix[i] == LETRA_ESPACIO) ? '\0' : (char)('a' + prefix[i]);
    if (wc < pc) return -1;
    if (wc > pc) return 1;
  }
  return 0;
}

static bool prefix_exists(const uint8_t *prefix, uint8_t len) {
  uint16_t lo = 0, hi = 2048;
  while (lo < hi) {
    const uint16_t mid = lo + ((hi - lo) >> 1);
    if (compare_word_prefix(mid, prefix, len) < 0) lo = mid + 1;
    else hi = mid;
  }
  return lo < 2048 && compare_word_prefix(lo, prefix, len) == 0;
}

static int16_t busca_palabra() {
  uint16_t lo = 0, hi = 2048;
  while (lo < hi) {
    const uint16_t mid = lo + ((hi - lo) >> 1);
    if (compare_word_prefix(mid, palabra, 4) < 0) lo = mid + 1;
    else hi = mid;
  }
  if (lo < 2048 && compare_word_prefix(lo, palabra, 4) == 0) return (int16_t)lo;
  return -1;
}

static void pone_siguiente_letra(int8_t dire) {
  // Iterative version: avoids recursion and 2048-word linear scans.
  for (uint8_t tries = 0; tries < 28; tries++) {
    if (letter_cont == LETRA_BORRA) return;

    if (letter_cont < 26 || (letter_cont == LETRA_ESPACIO && op_cont == 3)) {
      uint8_t prefix[4];
      for (uint8_t i = 0; i < op_cont; i++) prefix[i] = palabra[i];
      prefix[op_cont] = letter_cont;
      if (prefix_exists(prefix, op_cont + 1)) return;
    }

    if (dire > 0) {
      letter_cont++;
      if (letter_cont > LETRA_BORRA) letter_cont = 0;
    } else {
      if (letter_cont == 0) letter_cont = LETRA_BORRA;
      else letter_cont--;
    }
  }
}

static void clear_row(uint8_t row) {
  oled.setCursor(0, row);
  for (uint8_t i = 0; i < 16; i++) oled.write(' ');
}

static inline uint8_t entropy_bytes() {
  return seed_length == 24 ? 32 : 16;
}

static inline uint16_t entropy_bits() {
  return (uint16_t)entropy_bytes() << 3;
}

static inline uint8_t checksum_bit_count() {
  return seed_length == 24 ? 8 : 4;
}

void setup() {
  oled.begin(128, 32, sizeof(tiny4koled_init_128x32br), tiny4koled_init_128x32br);
  oled.clear();
  oled.setFont(FONT8X16);

  pinMode(BOTON_LEFT, INPUT_PULLUP);
  pinMode(BOTON_RIGHT, INPUT_PULLUP);
  pinMode(BOTON_ENTER, INPUT_PULLUP);

  // Minimal startup splash.
  oled.setCursor(0, PRIMER_RENGLON);
  oled.print("  SeedSplitter  ");
  oled.setCursor(0, SEGUNDO_RENGLON);
  oled.print("                ");
  oled.on();
  delay(2000);

  show_main_menu();
}

static uint8_t gmul(uint8_t a, uint8_t b) {
  uint8_t p = 0;
  while (a != 0 && b != 0) {
    if (b & 1) p ^= a;
    if (a & 0x80) a = (a << 1) ^ 0x1b;
    else a <<= 1;
    b >>= 1;
  }
  return p;
}

// Direct conversion, without the old 128/256-byte temporary bit arrays.
static void seed_to_bytes(const uint16_t *seed, uint8_t *values) {
  memset(values, 0, entropy_bytes());
  for (uint16_t bit = 0; bit < entropy_bits(); bit++) {
    const uint8_t v = (seed[bit / 11] >> (10 - (bit % 11))) & 1;
    values[bit >> 3] |= v << (7 - (bit & 7));
  }
}

static void bytes_to_seed(const uint8_t *values, uint16_t *seed, uint8_t checksum) {
  memset(seed, 0, seed_length * sizeof(uint16_t));
  for (uint16_t bit = 0; bit < entropy_bits(); bit++) {
    const uint8_t v = (values[bit >> 3] >> (7 - (bit & 7))) & 1;
    seed[bit / 11] |= ((uint16_t)v) << (10 - (bit % 11));
  }
  seed[seed_length - 1] |= checksum;
}

static bool valid_bip39_checksum() {
  uint8_t entropy[32];
  uint8_t digest[32];

  seed_to_bytes(seed_phrase, entropy);
  sha256.reset();
  sha256.update(entropy, entropy_bytes());
  sha256.finalize(digest, sizeof(digest));

  const uint8_t cs_bits = checksum_bit_count();
  const uint8_t expected = digest[0] >> (8 - cs_bits);
  const uint8_t mask = (uint8_t)((1U << cs_bits) - 1U);
  const uint8_t actual = (uint8_t)(seed_phrase[seed_length - 1] & mask);

  secure_zero(entropy, sizeof(entropy));
  secure_zero(digest, sizeof(digest));
  return actual == expected;
}

// Recover accepts only SeedSplitter shares: each share must be a valid
// BIP39 mnemonic, must carry one of the share identifiers 1..3, and the
// second share must be different from the first one.
static bool valid_recovery_share() {
  if (!valid_bip39_checksum()) return false;

  const uint8_t id = (uint8_t)(seed_phrase[seed_length - 1] & 0x03);
  if (id == 0) return false;

  if (seeds_cargadas == 1) {
    const uint8_t first_id =
        (uint8_t)(workspace.share[0][seed_length - 1] & 0x03);
    if (id == first_id) return false;
  }

  return true;
}

static uint8_t read_raw_buttons() {
  uint8_t state = 0;
  if (digitalRead(BOTON_RIGHT) == LOW) state |= RIGHT;
  if (digitalRead(BOTON_LEFT)  == LOW) state |= LEFT;
  if (digitalRead(BOTON_ENTER) == LOW) state |= ENTER;
  return state;
}

// Return only a new, debounced press event. Releases do not generate events,
// so one physical press causes exactly one UI action and one screen redraw.
static uint8_t read_button_event() {
  const uint8_t raw = read_raw_buttons();
  const uint16_t now = (uint16_t)millis();

  if (raw != raw_buttons_last) {
    raw_buttons_last = raw;
    debounce_since = now;
    return 0;
  }

  if (raw == stable_buttons || (uint16_t)(now - debounce_since) < 20) return 0;

  stable_buttons = raw;
  if (stable_buttons == RIGHT || stable_buttons == LEFT || stable_buttons == ENTER)
    return stable_buttons;
  return 0;
}

static inline void pad_line() {
  for (uint8_t i = oled.getCursorX() >> 3; i < 16; i++) oled.write(' ');
}

static void show_main_menu() {
  const uint8_t selected = op_cont % 4;

  // Fixed 2x16 layout. The words never move: the brackets occupy
  // reserved spaces immediately before/after the selected option.
  oled.setCursor(0, PRIMER_RENGLON);
  if (selected == MODE_SPLIT) oled.print("[Split] Recover ");
  else if (selected == MODE_RECOVER) oled.print(" Split [Recover]");
  else oled.print(" Split  Recover ");
  pad_line();

  oled.setCursor(0, SEGUNDO_RENGLON);
  if (selected == MODE_NEW_SEED) oled.print("[Generate] zpub ");
  else if (selected == MODE_ZPUB) oled.print(" Generate [zpub]");
  else oled.print(" Generate  zpub ");
  pad_line();

  oled.on();
}

static void show_length_menu() {
  oled.setCursor(0, PRIMER_RENGLON);
  oled.print("Largo de Seed:  ");
  oled.setCursor(0, SEGUNDO_RENGLON);
  if (op_cont % 2 == 0) oled.print("  [12]    24    ");
  else oled.print("   12    [24]   ");
  oled.on();
}

static void show_dice_input(bool pressed = false) {
  oled.setCursor(0, PRIMER_RENGLON);
  oled.print("Dado ");
  oled.print(dice_count + 1);
  oled.write('/');
  oled.print(DICE_ROLLS);
  pad_line();

  oled.setCursor(0, SEGUNDO_RENGLON);
  oled.print("Valor: ");
  if (dice_value == 7) oled.write('<');
  else oled.print(dice_value);

  if (pressed) oled.print(".");
  pad_line();
  oled.on();
}

static void show_share_word() {
  const uint8_t s = op_cont / seed_length;
  const uint8_t w = op_cont % seed_length;
  oled.setCursor(0, PRIMER_RENGLON);
  oled.print("Share ");
  oled.print(s + 1);
  oled.print(", Word ");
  oled.print(w + 1);
  pad_line();
  oled.setCursor(0, SEGUNDO_RENGLON);
  print_word(workspace.share[s][w]);
  pad_line();
  oled.on();
}

static void print_word_position(uint8_t index) {
  oled.print("Word ");
  if (index + 1 < 10) oled.write('0');
  oled.print(index + 1);
  oled.write('/');
  oled.print(seed_length);
}

static void show_seed_word() {
  oled.setCursor(0, PRIMER_RENGLON);
  print_word_position(op_cont);
  pad_line();
  oled.setCursor(0, SEGUNDO_RENGLON);
  print_word(seed_phrase[op_cont]);
  pad_line();
  oled.on();
}

static void show_word_input_state() {
  oled.setCursor(0, PRIMER_RENGLON);
  if (seleccion == MODE_SPLIT || seleccion == MODE_ZPUB) {
    print_word_position(word_cont);
  } else {
    if (seeds_cargadas == 0) oled.print("Seed 1, Word ");
    else oled.print("Seed 2, Word ");
    oled.print(word_cont + 1);
  }
  pad_line();

  // Repaint the already-confirmed prefix as well as the letter currently
  // being edited.  This keeps the input state self-contained whenever the
  // screen is redrawn.
  oled.setCursor(0, SEGUNDO_RENGLON);
  for (uint8_t i = 0; i < op_cont; i++) {
    if (palabra[i] == LETRA_ESPACIO) oled.write(' ');
    else oled.write('a' + palabra[i]);
  }

  oled.setCursor(op_cont * LARGO_LETRA, SEGUNDO_RENGLON);
  if (letter_cont == LETRA_BORRA) oled.write('<');
  else if (letter_cont == LETRA_ESPACIO) oled.write(' ');
  else oled.write('a' + letter_cont);
  oled.on();
}

static void show_zpub_progress() {
  oled.setCursor(0, PRIMER_RENGLON);
  oled.print("BIP84 watch-only");
  pad_line();
  oled.setCursor(0, SEGUNDO_RENGLON);
  oled.print("Calculando zpub");
  pad_line();
  oled.on();
}

static void show_invalid_seed() {
  oled.setCursor(0, PRIMER_RENGLON);
  oled.print("Seed invalida");
  pad_line();
  clear_row(SEGUNDO_RENGLON);
  oled.on();
}

static void show_zpub_error() {
  oled.setCursor(0, PRIMER_RENGLON);
  oled.print("Watch-only error");
  pad_line();
  oled.setCursor(0, SEGUNDO_RENGLON);
  oled.print("ENTER: menu");
  pad_line();
  oled.on();
}

static void generate_seed_from_dice() {
  uint8_t entropy[32];
  uint8_t cs;

  // Hash ASCII faces (e.g. "6152..."). This is easy to reproduce externally.
  sha256.reset();
  for (uint8_t i = 0; i < DICE_ROLLS; i++) {
    const uint8_t c = '0' + workspace.dice_rolls[i];
    sha256.update(&c, 1);
  }
  sha256.finalize(entropy, sizeof(entropy));

  // Both modes use 50 dice rolls (~129 bits of source entropy).
  // SHA-256 deterministically expands that source to 256 bits;
  // 12-word mode uses the first 128 bits, while 24-word mode
  // uses all 256 output bits. The 24-word mnemonic therefore still
  // has only the entropy supplied by the 50 physical rolls.
  sha256.reset();
  sha256.update(entropy, entropy_bytes());
  sha256.finalize(&cs, 1);
  cs >>= 8 - checksum_bit_count();
  bytes_to_seed(entropy, seed_phrase, cs);

  secure_zero(workspace.dice_rolls, sizeof(workspace.dice_rolls));
  secure_zero(entropy, sizeof(entropy));
}

static void show_selected_info() {
  oled.setCursor(0, PRIMER_RENGLON);
  if (seleccion == MODE_SPLIT) {
    oled.print("Divide ");
    oled.print(seed_length);
    oled.print(" pal.");
  } else if (seleccion == MODE_RECOVER) {
    oled.print("Recupera ");
    oled.print(seed_length);
    oled.print(" pal.");
  } else if (seleccion == MODE_NEW_SEED) {
    oled.print("Genera ");
    oled.print(seed_length);
    oled.print(" pal.");
  } else {
    oled.print("Ingresa ");
    oled.print(seed_length);
    oled.print(" pal.");
  }
  pad_line();

  oled.setCursor(0, SEGUNDO_RENGLON);
  if (seleccion == MODE_SPLIT) {
    oled.print("en 3 partes");
  } else if (seleccion == MODE_RECOVER) {
    oled.print("usando 2 partes");
  } else if (seleccion == MODE_NEW_SEED) {
    oled.print("tirando 50 dados");
  } else {
    oled.print("para zpub");
  }
  pad_line();
  oled.on();
}

static void start_selected_mode() {
  op_cont = word_cont = letter_cont = 0;
  memset(palabra, 0xff, sizeof(palabra));

  if (seleccion == MODE_NEW_SEED) {
    dice_count = 0;
    dice_value = 1;
    menu = DICE_INPUT;
    show_dice_input();
  } else {
    if (seleccion == MODE_ZPUB) {
      watch_zpub_ready = false;
      watch_zpub[0] = '\0';
    }
    clear_row(SEGUNDO_RENGLON);
    menu = WORD_INPUT;
    show_word_input_state();
  }
}

void loop() {
  if (menu == SHOW_ZPUB_QR) {
    update_watchonly_qr();
    return;
  }

  if (menu == INVALID_SEED) {
    return;
  }

  const uint8_t button_state = read_button_event();
  if (!button_state) return;

  switch (menu) {
    case MAIN_MENU:
      if (button_state == ENTER) {
        seleccion = op_cont % 4;
        menu = SEED_LENGTH;
        op_cont = 0;
        show_length_menu();
      } else if (button_state == LEFT) {
        op_cont = (op_cont + 1) % 4;  // next option
        show_main_menu();
      } else if (button_state == RIGHT) {
        op_cont = (op_cont + 3) % 4;  // previous option
        show_main_menu();
      }
      break;

    case SEED_LENGTH:
      if (button_state == ENTER) {
        seed_length = (op_cont % 2 == 0) ? 12 : 24;
        menu = INFO_SCREEN;
        show_selected_info();
      } else if (button_state == LEFT || button_state == RIGHT) {
        op_cont ^= 1;
        show_length_menu();
      }
      break;

    case INFO_SCREEN:
      // Any button closes the explanation and starts the selected action.
      start_selected_mode();
      break;

    case DICE_INPUT:
      if (button_state == LEFT) {
        dice_value++;
        if (dice_value > 7) dice_value = 1;
      } else if (button_state == RIGHT) {
        if (dice_value <= 1) dice_value = 7;
        else dice_value--;
      } else if (button_state == ENTER) {
        if (dice_value == 7) {
          if (dice_count > 0) {
            dice_count--;
            dice_value = workspace.dice_rolls[dice_count];
          }
        } else {
          show_dice_input(true);
          delay(200);
          workspace.dice_rolls[dice_count++] = dice_value;
          dice_value = 1;
          if (dice_count == DICE_ROLLS) {
            generate_seed_from_dice();
            // Generate stops at the BIP39 words. Watch-only export is a
            // separate zpub flow that requires re-entering the mnemonic.
            op_cont = 0;
            menu = SHOW_SEED;
            show_seed_word();
            break;
          }
        }
      }
      show_dice_input();
      break;

    case SHOW_SHARES:
      if (button_state == LEFT && op_cont < 3 * seed_length - 1) {
        op_cont++;
        show_share_word();
      } else if (button_state == RIGHT && op_cont > 0) {
        op_cont--;
        show_share_word();
      }
      break;

    case SHOW_SEED:
      // Seeds shown by Generate and Recover are display-only.
      // Generate intentionally does not derive or expose a watch-only QR.
      if (button_state == LEFT && op_cont < seed_length - 1) {
        op_cont++;
        show_seed_word();
      } else if (button_state == RIGHT && op_cont > 0) {
        op_cont--;
        show_seed_word();
      }
      break;

    case ZPUB_ERROR:
      if (button_state == ENTER) {
        secure_zero(seed_phrase, sizeof(seed_phrase));
        secure_zero(watch_zpub, sizeof(watch_zpub));
        watch_zpub_ready = false;
        op_cont = MODE_ZPUB;
        menu = MAIN_MENU;
        show_main_menu();
      }
      break;

    case WORD_INPUT:
      if (button_state == LEFT) {
        letter_cont++;
        if (letter_cont > LETRA_BORRA) letter_cont = 0;
        pone_siguiente_letra(1);

      } else if (button_state == RIGHT) {
        if (letter_cont == 0) letter_cont = LETRA_BORRA;
        else letter_cont--;
        pone_siguiente_letra(-1);

      } else if (button_state == ENTER) {
        if (letter_cont == LETRA_BORRA) {
          if (op_cont > 0) {
            palabra[op_cont] = 0xff;
            op_cont--;
            letter_cont = palabra[op_cont];
            palabra[op_cont] = 0xff;
            oled.setCursor(op_cont * LARGO_LETRA, SEGUNDO_RENGLON);
            oled.write(' ');
            oled.write(' ');
            oled.on();
          } else if (word_cont > 0) {
            word_cont--;
            op_cont = 3;
            palabra[0] = word_char(seed_phrase[word_cont], 0) - 'a';
            palabra[1] = word_char(seed_phrase[word_cont], 1) - 'a';
            palabra[2] = word_char(seed_phrase[word_cont], 2) - 'a';
            const char c4 = word_char(seed_phrase[word_cont], 3);
            palabra[3] = c4 ? c4 - 'a' : LETRA_ESPACIO;

            oled.setCursor(0, SEGUNDO_RENGLON);
            oled.write('a' + palabra[0]);
            oled.write('a' + palabra[1]);
            oled.write('a' + palabra[2]);
            if (palabra[3] == LETRA_ESPACIO) oled.write(' ');
            else oled.write('a' + palabra[3]);
            oled.on();
            letter_cont = palabra[op_cont];
            palabra[op_cont] = 0xff;
          }
        } else {
          palabra[op_cont] = letter_cont;
          op_cont++;
          if (op_cont < 4) {
            letter_cont = 0;
            pone_siguiente_letra(1);
          }
        }

        if (op_cont == 4) {
          const int16_t pos_word = busca_palabra();
          if (pos_word >= 0) {
            oled.setCursor(0, SEGUNDO_RENGLON);
            print_word(pos_word);
            oled.print('.');
            oled.on();
            seed_phrase[word_cont++] = pos_word;
          }
          op_cont = 0;
          letter_cont = 0;
          delay(700);
          clear_row(SEGUNDO_RENGLON);
          memset(palabra, 0xff, sizeof(palabra));
        }

        if (word_cont == seed_length) {
          if (seleccion == MODE_SPLIT) {
            if (!valid_bip39_checksum()) {
              show_invalid_seed();
              menu = INVALID_SEED;
              break;
            }

            split();
            menu = SHOW_SHARES;
            op_cont = word_cont = letter_cont = 0;
            show_share_word();
            break;
          }

          if (seleccion == MODE_ZPUB) {
            if (!valid_bip39_checksum()) {
              show_invalid_seed();
              menu = INVALID_SEED;
              break;
            }

            show_zpub_progress();
            if (!generate_watchonly_zpub()) {
              show_zpub_error();
              menu = ZPUB_ERROR;
              break;
            }

            // Only the public zpub is needed once derivation succeeds.
            // Erase the entered mnemonic before entering the terminal QR screen.
            secure_zero(seed_phrase, sizeof(seed_phrase));
            enter_watchonly_qr();
            break;
          }

          // Recover validates each entered share before accepting it.  Invalid
          // input stops here instead of attempting a reconstruction.
          if (!valid_recovery_share()) {
            show_invalid_seed();
            menu = INVALID_SEED;
            break;
          }

          memcpy(workspace.share[seeds_cargadas], seed_phrase,
                 seed_length * sizeof(uint16_t));
          seeds_cargadas++;
          if (seeds_cargadas == 2) {
            recover();
            menu = SHOW_SEED;
            op_cont = word_cont = letter_cont = 0;
            show_seed_word();
            break;
          }
          op_cont = word_cont = letter_cont = 0;
        }
      }

      show_word_input_state();
      break;
  }
}

void split() {
  uint8_t seed_bytes[32];
  uint8_t share_bytes[3][32];
  uint8_t random_values[32];
  uint8_t cs0, cs1, cs2;

  seed_to_bytes(seed_phrase, seed_bytes);
  memcpy(random_values, seed_bytes, entropy_bytes());

  while (true) {
    sha256.reset();
    sha256.update(random_values, entropy_bytes());
    sha256.finalize(random_values, entropy_bytes());

    for (uint8_t i = 0; i < entropy_bytes(); i++) {
      uint8_t m = random_values[i];
      if (m == 0) m = 0x0f;
      share_bytes[0][i] = gmul(m, 1) ^ seed_bytes[i];
      share_bytes[1][i] = gmul(m, 2) ^ seed_bytes[i];
      share_bytes[2][i] = gmul(m, 3) ^ seed_bytes[i];
    }

    sha256.reset();
    sha256.update(share_bytes[0], entropy_bytes());
    sha256.finalize(&cs0, 1);
    sha256.reset();
    sha256.update(share_bytes[1], entropy_bytes());
    sha256.finalize(&cs1, 1);
    sha256.reset();
    sha256.update(share_bytes[2], entropy_bytes());
    sha256.finalize(&cs2, 1);

    cs0 >>= 8 - checksum_bit_count();
    cs1 >>= 8 - checksum_bit_count();
    cs2 >>= 8 - checksum_bit_count();
    if (cs0 % 4 == 1 && cs1 % 4 == 2 && cs2 % 4 == 3) break;
  }

  bytes_to_seed(share_bytes[0], workspace.share[0], cs0);
  bytes_to_seed(share_bytes[1], workspace.share[1], cs1);
  bytes_to_seed(share_bytes[2], workspace.share[2], cs2);

  secure_zero(seed_bytes, sizeof(seed_bytes));
  secure_zero(share_bytes, sizeof(share_bytes));
  secure_zero(random_values, sizeof(random_values));
}

void recover() {
  static const uint8_t v_table[4][4] = {
    {0,   0,   0,   0},
    {0,   0, 247, 140},
    {0, 246,   0,   3},
    {0, 141,   2,   0}
  };

  const uint8_t x0 = workspace.share[0][seed_length - 1] % 4;
  const uint8_t x1 = workspace.share[1][seed_length - 1] % 4;
  const uint8_t v0 = v_table[x0][x1];
  const uint8_t v1 = v_table[x1][x0];

  uint8_t share_bytes[2][32];
  uint8_t seed_bytes[32];
  uint8_t cs;

  seed_to_bytes(workspace.share[0], share_bytes[0]);
  seed_to_bytes(workspace.share[1], share_bytes[1]);
  for (uint8_t i = 0; i < entropy_bytes(); i++)
    seed_bytes[i] = gmul(share_bytes[0][i], v0) ^ gmul(share_bytes[1][i], v1);

  sha256.reset();
  sha256.update(seed_bytes, entropy_bytes());
  sha256.finalize(&cs, 1);
  cs >>= 8 - checksum_bit_count();
  bytes_to_seed(seed_bytes, seed_phrase, cs);

  secure_zero(share_bytes, sizeof(share_bytes));
  secure_zero(seed_bytes, sizeof(seed_bytes));
}
