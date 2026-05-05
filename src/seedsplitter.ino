#include <Tiny4kOLED.h>
#include <SHA256.h>

#define OSPLITORECOVER 0
#define CARGA_PALABRA 1
#define MENU_RECOVER 2
#define SHOW_SHARES 3
#define O12O24 4
#define OPCION_SPLIT 0
#define OPCION_RECOVER 1

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

SHA256 sha256;
byte menu = OSPLITORECOVER;
byte palabra[4] = { 0xff, 0xff, 0xff, 0xff };
byte op_cont = 0;
byte word_cont = 0;
byte letter_cont = 0;
byte old_button_state = -1;
byte normal_state_left, normal_state_right, normal_state_enter;
uint32_t seed_phrase[24];
uint32_t share[3][24];
int32_t seed_length, number_of_bits, number_of_bytes, checksum_bits, seleccion;
byte seeds_cargadas = 0;

const char* word_list[] = { "abandon", "ability", "able", "about", "above", "absent", "absorb", "abstract", "absurd", "abuse", "access", "accident", "account", "accuse", "achieve", "acid", "acoustic", "acquire", "across", "act", "action", "actor", "actress", "actual", "adapt", "add", "addict", "address", "adjust", "admit", "adult", "advance", "advice", "aerobic", "affair", "afford", "afraid", "again", "age", "agent", "agree", "ahead", "aim", "air", "airport", "aisle", "alarm", "album", "alcohol", "alert", "alien", "all", "alley", "allow", "almost", "alone", "alpha", "already", "also", "alter", "always", "amateur", "amazing", "among", "amount", "amused", "analyst", "anchor", "ancient", "anger", "angle", "angry", "animal", "ankle", "announce", "annual", "another", "answer", "antenna", "antique", "anxiety", "any", "apart", "apology", "appear", "apple", "approve", "april", "arch", "arctic", "area", "arena", "argue", "arm", "armed", "armor", "army", "around", "arrange", "arrest", "arrive", "arrow", "art", "artefact", "artist", "artwork", "ask", "aspect", "assault", "asset", "assist", "assume", "asthma", "athlete", "atom", "attack", "attend", "attitude", "attract", "auction", "audit", "august", "aunt", "author", "auto", "autumn", "average", "avocado", "avoid", "awake", "aware", "away", "awesome", "awful", "awkward", "axis", "baby", "bachelor", "bacon", "badge", "bag", "balance", "balcony", "ball", "bamboo", "banana", "banner", "bar", "barely", "bargain", "barrel", "base", "basic", "basket", "battle", "beach", "bean", "beauty", "because", "become", "beef", "before", "begin", "behave", "behind", "believe", "below", "belt", "bench", "benefit", "best", "betray", "better", "between", "beyond", "bicycle", "bid", "bike", "bind", "biology", "bird", "birth", "bitter", "black", "blade", "blame", "blanket", "blast", "bleak", "bless", "blind", "blood", "blossom", "blouse", "blue", "blur", "blush", "board", "boat", "body", "boil", "bomb", "bone", "bonus", "book", "boost", "border", "boring", "borrow", "boss", "bottom", "bounce", "box", "boy", "bracket", "brain", "brand", "brass", "brave", "bread", "breeze", "brick", "bridge", "brief", "bright", "bring", "brisk", "broccoli", "broken", "bronze", "broom", "brother", "brown", "brush", "bubble", "buddy", "budget", "buffalo", "build", "bulb", "bulk", "bullet", "bundle", "bunker", "burden", "burger", "burst", "bus", "business", "busy", "butter", "buyer", "buzz", "cabbage", "cabin", "cable", "cactus", "cage", "cake", "call", "calm", "camera", "camp", "can", "canal", "cancel", "candy", "cannon", "canoe", "canvas", "canyon", "capable", "capital", "captain", "car", "carbon", "card", "cargo", "carpet", "carry", "cart", "case", "cash", "casino", "castle", "casual", "cat", "catalog", "catch", "category", "cattle", "caught", "cause", "caution", "cave", "ceiling", "celery", "cement", "census", "century", "cereal", "certain", "chair", "chalk", "champion", "change", "chaos", "chapter", "charge", "chase", "chat", "cheap", "check", "cheese", "chef", "cherry", "chest", "chicken", "chief", "child", "chimney", "choice", "choose", "chronic", "chuckle", "chunk", "churn", "cigar", "cinnamon", "circle", "citizen", "city", "civil", "claim", "clap", "clarify", "claw", "clay", "clean", "clerk", "clever", "click", "client", "cliff", "climb", "clinic", "clip", "clock", "clog", "close", "cloth", "cloud", "clown", "club", "clump", "cluster", "clutch", "coach", "coast", "coconut", "code", "coffee", "coil", "coin", "collect", "color", "column", "combine", "come", "comfort", "comic", "common", "company", "concert", "conduct", "confirm", "congress", "connect", "consider", "control", "convince", "cook", "cool", "copper", "copy", "coral", "core", "corn", "correct", "cost", "cotton", "couch", "country", "couple", "course", "cousin", "cover", "coyote", "crack", "cradle", "craft", "cram", "crane", "crash", "crater", "crawl", "crazy", "cream", "credit", "creek", "crew", "cricket", "crime", "crisp", "critic", "crop", "cross", "crouch", "crowd", "crucial", "cruel", "cruise", "crumble", "crunch", "crush", "cry", "crystal", "cube", "culture", "cup", "cupboard", "curious", "current", "curtain", "curve", "cushion", "custom", "cute", "cycle", "dad", "damage", "damp", "dance", "danger", "daring", "dash", "daughter", "dawn", "day", "deal", "debate", "debris", "decade", "december", "decide", "decline", "decorate", "decrease", "deer", "defense", "define", "defy", "degree", "delay", "deliver", "demand", "demise", "denial", "dentist", "deny", "depart", "depend", "deposit", "depth", "deputy", "derive", "describe", "desert", "design", "desk", "despair", "destroy", "detail", "detect", "develop", "device", "devote", "diagram", "dial", "diamond", "diary", "dice", "diesel", "diet", "differ", "digital", "dignity", "dilemma", "dinner", "dinosaur", "direct", "dirt", "disagree", "discover", "disease", "dish", "dismiss", "disorder", "display", "distance", "divert", "divide", "divorce", "dizzy", "doctor", "document", "dog", "doll", "dolphin", "domain", "donate", "donkey", "donor", "door", "dose", "double", "dove", "draft", "dragon", "drama", "drastic", "draw", "dream", "dress", "drift", "drill", "drink", "drip", "drive", "drop", "drum", "dry", "duck", "dumb", "dune", "during", "dust", "dutch", "duty", "dwarf", "dynamic", "eager", "eagle", "early", "earn", "earth", "easily", "east", "easy", "echo", "ecology", "economy", "edge", "edit", "educate", "effort", "egg", "eight", "either", "elbow", "elder", "electric", "elegant", "element", "elephant", "elevator", "elite", "else", "embark", "embody", "embrace", "emerge", "emotion", "employ", "empower", "empty", "enable", "enact", "end", "endless", "endorse", "enemy", "energy", "enforce", "engage", "engine", "enhance", "enjoy", "enlist", "enough", "enrich", "enroll", "ensure", "enter", "entire", "entry", "envelope", "episode", "equal", "equip", "era", "erase", "erode", "erosion", "error", "erupt", "escape", "essay", "essence", "estate", "eternal", "ethics", "evidence", "evil", "evoke", "evolve", "exact", "example", "excess", "exchange", "excite", "exclude", "excuse", "execute", "exercise", "exhaust", "exhibit", "exile", "exist", "exit", "exotic", "expand", "expect", "expire", "explain", "expose", "express", "extend", "extra", "eye", "eyebrow", "fabric", "face", "faculty", "fade", "faint", "faith", "fall", "false", "fame", "family", "famous", "fan", "fancy", "fantasy", "farm", "fashion", "fat", "fatal", "father", "fatigue", "fault", "favorite", "feature", "february", "federal", "fee", "feed", "feel", "female", "fence", "festival", "fetch", "fever", "few", "fiber", "fiction", "field", "figure", "file", "film", "filter", "final", "find", "fine", "finger", "finish", "fire", "firm", "first", "fiscal", "fish", "fit", "fitness", "fix", "flag", "flame", "flash", "flat", "flavor", "flee", "flight", "flip", "float", "flock", "floor", "flower", "fluid", "flush", "fly", "foam", "focus", "fog", "foil", "fold", "follow", "food", "foot", "force", "forest", "forget", "fork", "fortune", "forum", "forward", "fossil", "foster", "found", "fox", "fragile", "frame", "frequent", "fresh", "friend", "fringe", "frog", "front", "frost", "frown", "frozen", "fruit", "fuel", "fun", "funny", "furnace", "fury", "future", "gadget", "gain", "galaxy", "gallery", "game", "gap", "garage", "garbage", "garden", "garlic", "garment", "gas", "gasp", "gate", "gather", "gauge", "gaze", "general", "genius", "genre", "gentle", "genuine", "gesture", "ghost", "giant", "gift", "giggle", "ginger", "giraffe", "girl", "give", "glad", "glance", "glare", "glass", "glide", "glimpse", "globe", "gloom", "glory", "glove", "glow", "glue", "goat", "goddess", "gold", "good", "goose", "gorilla", "gospel", "gossip", "govern", "gown", "grab", "grace", "grain", "grant", "grape", "grass", "gravity", "great", "green", "grid", "grief", "grit", "grocery", "group", "grow", "grunt", "guard", "guess", "guide", "guilt", "guitar", "gun", "gym", "habit", "hair", "half", "hammer", "hamster", "hand", "happy", "harbor", "hard", "harsh", "harvest", "hat", "have", "hawk", "hazard", "head", "health", "heart", "heavy", "hedgehog", "height", "hello", "helmet", "help", "hen", "hero", "hidden", "high", "hill", "hint", "hip", "hire", "history", "hobby", "hockey", "hold", "hole", "holiday", "hollow", "home", "honey", "hood", "hope", "horn", "horror", "horse", "hospital", "host", "hotel", "hour", "hover", "hub", "huge", "human", "humble", "humor", "hundred", "hungry", "hunt", "hurdle", "hurry", "hurt", "husband", "hybrid", "ice", "icon", "idea", "identify", "idle", "ignore", "ill", "illegal", "illness", "image", "imitate", "immense", "immune", "impact", "impose", "improve", "impulse", "inch", "include", "income", "increase", "index", "indicate", "indoor", "industry", "infant", "inflict", "inform", "inhale", "inherit", "initial", "inject", "injury", "inmate", "inner", "innocent", "input", "inquiry", "insane", "insect", "inside", "inspire", "install", "intact", "interest", "into", "invest", "invite", "involve", "iron", "island", "isolate", "issue", "item", "ivory", "jacket", "jaguar", "jar", "jazz", "jealous", "jeans", "jelly", "jewel", "job", "join", "joke", "journey", "joy", "judge", "juice", "jump", "jungle", "junior", "junk", "just", "kangaroo", "keen", "keep", "ketchup", "key", "kick", "kid", "kidney", "kind", "kingdom", "kiss", "kit", "kitchen", "kite", "kitten", "kiwi", "knee", "knife", "knock", "know", "lab", "label", "labor", "ladder", "lady", "lake", "lamp", "language", "laptop", "large", "later", "latin", "laugh", "laundry", "lava", "law", "lawn", "lawsuit", "layer", "lazy", "leader", "leaf", "learn", "leave", "lecture", "left", "leg", "legal", "legend", "leisure", "lemon", "lend", "length", "lens", "leopard", "lesson", "letter", "level", "liar", "liberty", "library", "license", "life", "lift", "light", "like", "limb", "limit", "link", "lion", "liquid", "list", "little", "live", "lizard", "load", "loan", "lobster", "local", "lock", "logic", "lonely", "long", "loop", "lottery", "loud", "lounge", "love", "loyal", "lucky", "luggage", "lumber", "lunar", "lunch", "luxury", "lyrics", "machine", "mad", "magic", "magnet", "maid", "mail", "main", "major", "make", "mammal", "man", "manage", "mandate", "mango", "mansion", "manual", "maple", "marble", "march", "margin", "marine", "market", "marriage", "mask", "mass", "master", "match", "material", "math", "matrix", "matter", "maximum", "maze", "meadow", "mean", "measure", "meat", "mechanic", "medal", "media", "melody", "melt", "member", "memory", "mention", "menu", "mercy", "merge", "merit", "merry", "mesh", "message", "metal", "method", "middle", "midnight", "milk", "million", "mimic", "mind", "minimum", "minor", "minute", "miracle", "mirror", "misery", "miss", "mistake", "mix", "mixed", "mixture", "mobile", "model", "modify", "mom", "moment", "monitor", "monkey", "monster", "month", "moon", "moral", "more", "morning", "mosquito", "mother", "motion", "motor", "mountain", "mouse", "move", "movie", "much", "muffin", "mule", "multiply", "muscle", "museum", "mushroom", "music", "must", "mutual", "myself", "mystery", "myth", "naive", "name", "napkin", "narrow", "nasty", "nation", "nature", "near", "neck", "need", "negative", "neglect", "neither", "nephew", "nerve", "nest", "net", "network", "neutral", "never", "news", "next", "nice", "night", "noble", "noise", "nominee", "noodle", "normal", "north", "nose", "notable", "note", "nothing", "notice", "novel", "now", "nuclear", "number", "nurse", "nut", "oak", "obey", "object", "oblige", "obscure", "observe", "obtain", "obvious", "occur", "ocean", "october", "odor", "off", "offer", "office", "often", "oil", "okay", "old", "olive", "olympic", "omit", "once", "one", "onion", "online", "only", "open", "opera", "opinion", "oppose", "option", "orange", "orbit", "orchard", "order", "ordinary", "organ", "orient", "original", "orphan", "ostrich", "other", "outdoor", "outer", "output", "outside", "oval", "oven", "over", "own", "owner", "oxygen", "oyster", "ozone", "pact", "paddle", "page", "pair", "palace", "palm", "panda", "panel", "panic", "panther", "paper", "parade", "parent", "park", "parrot", "party", "pass", "patch", "path", "patient", "patrol", "pattern", "pause", "pave", "payment", "peace", "peanut", "pear", "peasant", "pelican", "pen", "penalty", "pencil", "people", "pepper", "perfect", "permit", "person", "pet", "phone", "photo", "phrase", "physical", "piano", "picnic", "picture", "piece", "pig", "pigeon", "pill", "pilot", "pink", "pioneer", "pipe", "pistol", "pitch", "pizza", "place", "planet", "plastic", "plate", "play", "please", "pledge", "pluck", "plug", "plunge", "poem", "poet", "point", "polar", "pole", "police", "pond", "pony", "pool", "popular", "portion", "position", "possible", "post", "potato", "pottery", "poverty", "powder", "power", "practice", "praise", "predict", "prefer", "prepare", "present", "pretty", "prevent", "price", "pride", "primary", "print", "priority", "prison", "private", "prize", "problem", "process", "produce", "profit", "program", "project", "promote", "proof", "property", "prosper", "protect", "proud", "provide", "public", "pudding", "pull", "pulp", "pulse", "pumpkin", "punch", "pupil", "puppy", "purchase", "purity", "purpose", "purse", "push", "put", "puzzle", "pyramid", "quality", "quantum", "quarter", "question", "quick", "quit", "quiz", "quote", "rabbit", "raccoon", "race", "rack", "radar", "radio", "rail", "rain", "raise", "rally", "ramp", "ranch", "random", "range", "rapid", "rare", "rate", "rather", "raven", "raw", "razor", "ready", "real", "reason", "rebel", "rebuild", "recall", "receive", "recipe", "record", "recycle", "reduce", "reflect", "reform", "refuse", "region", "regret", "regular", "reject", "relax", "release", "relief", "rely", "remain", "remember", "remind", "remove", "render", "renew", "rent", "reopen", "repair", "repeat", "replace", "report", "require", "rescue", "resemble", "resist", "resource", "response", "result", "retire", "retreat", "return", "reunion", "reveal", "review", "reward", "rhythm", "rib", "ribbon", "rice", "rich", "ride", "ridge", "rifle", "right", "rigid", "ring", "riot", "ripple", "risk", "ritual", "rival", "river", "road", "roast", "robot", "robust", "rocket", "romance", "roof", "rookie", "room", "rose", "rotate", "rough", "round", "route", "royal", "rubber", "rude", "rug", "rule", "run", "runway", "rural", "sad", "saddle", "sadness", "safe", "sail", "salad", "salmon", "salon", "salt", "salute", "same", "sample", "sand", "satisfy", "satoshi", "sauce", "sausage", "save", "say", "scale", "scan", "scare", "scatter", "scene", "scheme", "school", "science", "scissors", "scorpion", "scout", "scrap", "screen", "script", "scrub", "sea", "search", "season", "seat", "second", "secret", "section", "security", "seed", "seek", "segment", "select", "sell", "seminar", "senior", "sense", "sentence", "series", "service", "session", "settle", "setup", "seven", "shadow", "shaft", "shallow", "share", "shed", "shell", "sheriff", "shield", "shift", "shine", "ship", "shiver", "shock", "shoe", "shoot", "shop", "short", "shoulder", "shove", "shrimp", "shrug", "shuffle", "shy", "sibling", "sick", "side", "siege", "sight", "sign", "silent", "silk", "silly", "silver", "similar", "simple", "since", "sing", "siren", "sister", "situate", "six", "size", "skate", "sketch", "ski", "skill", "skin", "skirt", "skull", "slab", "slam", "sleep", "slender", "slice", "slide", "slight", "slim", "slogan", "slot", "slow", "slush", "small", "smart", "smile", "smoke", "smooth", "snack", "snake", "snap", "sniff", "snow", "soap", "soccer", "social", "sock", "soda", "soft", "solar", "soldier", "solid", "solution", "solve", "someone", "song", "soon", "sorry", "sort", "soul", "sound", "soup", "source", "south", "space", "spare", "spatial", "spawn", "speak", "special", "speed", "spell", "spend", "sphere", "spice", "spider", "spike", "spin", "spirit", "split", "spoil", "sponsor", "spoon", "sport", "spot", "spray", "spread", "spring", "spy", "square", "squeeze", "squirrel", "stable", "stadium", "staff", "stage", "stairs", "stamp", "stand", "start", "state", "stay", "steak", "steel", "stem", "step", "stereo", "stick", "still", "sting", "stock", "stomach", "stone", "stool", "story", "stove", "strategy", "street", "strike", "strong", "struggle", "student", "stuff", "stumble", "style", "subject", "submit", "subway", "success", "such", "sudden", "suffer", "sugar", "suggest", "suit", "summer", "sun", "sunny", "sunset", "super", "supply", "supreme", "sure", "surface", "surge", "surprise", "surround", "survey", "suspect", "sustain", "swallow", "swamp", "swap", "swarm", "swear", "sweet", "swift", "swim", "swing", "switch", "sword", "symbol", "symptom", "syrup", "system", "table", "tackle", "tag", "tail", "talent", "talk", "tank", "tape", "target", "task", "taste", "tattoo", "taxi", "teach", "team", "tell", "ten", "tenant", "tennis", "tent", "term", "test", "text", "thank", "that", "theme", "then", "theory", "there", "they", "thing", "this", "thought", "three", "thrive", "throw", "thumb", "thunder", "ticket", "tide", "tiger", "tilt", "timber", "time", "tiny", "tip", "tired", "tissue", "title", "toast", "tobacco", "today", "toddler", "toe", "together", "toilet", "token", "tomato", "tomorrow", "tone", "tongue", "tonight", "tool", "tooth", "top", "topic", "topple", "torch", "tornado", "tortoise", "toss", "total", "tourist", "toward", "tower", "town", "toy", "track", "trade", "traffic", "tragic", "train", "transfer", "trap", "trash", "travel", "tray", "treat", "tree", "trend", "trial", "tribe", "trick", "trigger", "trim", "trip", "trophy", "trouble", "truck", "true", "truly", "trumpet", "trust", "truth", "try", "tube", "tuition", "tumble", "tuna", "tunnel", "turkey", "turn", "turtle", "twelve", "twenty", "twice", "twin", "twist", "two", "type", "typical", "ugly", "umbrella", "unable", "unaware", "uncle", "uncover", "under", "undo", "unfair", "unfold", "unhappy", "uniform", "unique", "unit", "universe", "unknown", "unlock", "until", "unusual", "unveil", "update", "upgrade", "uphold", "upon", "upper", "upset", "urban", "urge", "usage", "use", "used", "useful", "useless", "usual", "utility", "vacant", "vacuum", "vague", "valid", "valley", "valve", "van", "vanish", "vapor", "various", "vast", "vault", "vehicle", "velvet", "vendor", "venture", "venue", "verb", "verify", "version", "very", "vessel", "veteran", "viable", "vibrant", "vicious", "victory", "video", "view", "village", "vintage", "violin", "virtual", "virus", "visa", "visit", "visual", "vital", "vivid", "vocal", "voice", "void", "volcano", "volume", "vote", "voyage", "wage", "wagon", "wait", "walk", "wall", "walnut", "want", "warfare", "warm", "warrior", "wash", "wasp", "waste", "water", "wave", "way", "wealth", "weapon", "wear", "weasel", "weather", "web", "wedding", "weekend", "weird", "welcome", "west", "wet", "whale", "what", "wheat", "wheel", "when", "where", "whip", "whisper", "wide", "width", "wife", "wild", "will", "win", "window", "wine", "wing", "wink", "winner", "winter", "wire", "wisdom", "wise", "wish", "witness", "wolf", "woman", "wonder", "wood", "wool", "word", "work", "world", "worry", "worth", "wrap", "wreck", "wrestle", "wrist", "write", "wrong", "yard", "year", "yellow", "you", "young", "youth", "zebra", "zero", "zone", "zoo" };


void setup() {
  oled.begin(128, 32, sizeof(tiny4koled_init_128x32br), tiny4koled_init_128x32br);
  oled.clear();
  oled.setFont(FONT8X16);

  pinMode(BOTON_LEFT, INPUT_PULLUP);
  pinMode(BOTON_RIGHT, INPUT_PULLUP);
  pinMode(BOTON_ENTER, INPUT_PULLUP);
  normal_state_left = digitalRead(BOTON_LEFT);
  normal_state_right = digitalRead(BOTON_RIGHT);
  normal_state_enter = digitalRead(BOTON_ENTER);
}

static byte gadd(byte a, byte b) {
  return a ^ b;
}

static byte gmul(byte a, byte b) {
  byte p = 0;
  while (a != 0 && b != 0) {
    if (b & 1) p ^= a;
    if (a & 0x80) a = (a << 1) ^ 0x11b;
    else a <<= 1;
    b >>= 1;
  }
  return p;
}

static void seed_to_bytes(uint32_t* seed, byte* values) {
  byte data[number_of_bits];
  for (int w = 0; w < seed_length; w++)
    for (int i = 0; i < 11; i++)
      if (w * 11 + i < number_of_bits) data[w * 11 + i] = ((seed[w] >> (10 - i)) & 1);
  for (int i = 0; i < number_of_bytes; i++) {
    values[i] = 0;
    for (int j = 0; j < 8; j++) {
      values[i] |= data[i * 8 + j] << (7 - j);
    }
  }
}

static void bytes_to_seed(byte* values, uint32_t* seed, byte checksum) {
  byte data[number_of_bits];
  for (int w = 0; w < number_of_bytes; w++)
    for (int i = 0; i < 8; i++)
      data[w * 8 + i] = (values[w] >> (7 - i)) & 1;
  for (int i = 0; i < seed_length; i++) {
    seed[i] = 0;
    for (int j = 0; j < 11; j++) {
      if (i * 11 + j < number_of_bits) seed[i] |= data[i * 11 + j] << (10 - j);
    }
  }
  seed[seed_length - 1] |= checksum;
}

static int busca_palabra() {
  for (int i = 0; i < 2048; i++) {
    if (word_list[i][0] == 'a' + palabra[0])
      if (word_list[i][1] == 'a' + palabra[1])
        if (word_list[i][2] == 'a' + palabra[2]) {
          if (palabra[3] == LETRA_ESPACIO) return i;
          if (word_list[i][3] == 'a' + palabra[3]) return i;
        }
  }
  return -1;
}

static void pone_siguiente_letra(char dire) {
  if (letter_cont == LETRA_BORRA) return;
  if (palabra[0] == 0xff) {
    if (letter_cont == LETRA_ESPACIO) letter_cont += dire;
    return;
  } else if (palabra[1] == 0xff) {
    for (int i = 0; i < 2048; i++)
      if (word_list[i][0] == palabra[0] + 'a')
        if (word_list[i][1] == letter_cont + 'a') return;
  } else if (palabra[2] == 0xff) {
    for (int i = 0; i < 2048; i++)
      if (word_list[i][0] == palabra[0] + 'a')
        if (word_list[i][1] == palabra[1] + 'a')
          if (word_list[i][2] == letter_cont + 'a') return;
  } else if (palabra[3] == 0xff) {
    for (int i = 0; i < 2048; i++) {
      if (word_list[i][0] == palabra[0] + 'a')
        if (word_list[i][1] == palabra[1] + 'a')
          if (word_list[i][2] == palabra[2] + 'a') {
            byte l = word_list[i][3];
            if (l == '\0' && letter_cont == LETRA_ESPACIO) return;
            if (l == letter_cont + 'a') return;
          }
    }
  }
  letter_cont += dire;
  pone_siguiente_letra(dire);
}


static int read_buttons() {
  byte boton_l = 0;
  byte boton_r = 0;
  byte boton_e = 0;
  for (int i = 0; i < 50; i++) {
    if (digitalRead(BOTON_LEFT) != normal_state_left) boton_l = 1;
    if (digitalRead(BOTON_RIGHT) != normal_state_right) boton_r = 1;
    if (digitalRead(BOTON_ENTER) != normal_state_enter) boton_e = 1;
    delay(1);
  }
  if (boton_r + boton_l + boton_e == 1) {
    if (boton_e == 1) return ENTER;
    if (boton_r == 1) return RIGHT;
    if (boton_l == 1) return LEFT;
  }
  return 0;
}

void loop() {
  int button_state = read_buttons();

  if (menu == OSPLITORECOVER && button_state != old_button_state) {
    old_button_state = button_state;
    if (button_state == ENTER) {
      if (op_cont % 2 == 0) seleccion = OPCION_SPLIT;
      else seleccion = OPCION_RECOVER;
      menu = O12O24;
      op_cont = 0;
    } else if (button_state == LEFT || button_state == RIGHT) op_cont++;
    if (menu == OSPLITORECOVER) {
      oled.setCursor(0, PRIMER_RENGLON);
      oled.print("Seleccionar:  ");
      oled.setCursor(0, SEGUNDO_RENGLON);
      if (op_cont % 2 == 0) oled.print("[Split] Recover ");
      else oled.print(" Split [Recover]");
      oled.on();
    }
  }

  if (menu == O12O24 && button_state != old_button_state) {
    old_button_state = button_state;
    if (button_state == ENTER) {
      if (op_cont % 2 == 0) {
        seed_length = 12;
        number_of_bits = 128;
        number_of_bytes = 16;
      } else {
        seed_length = 24;
        number_of_bits = 256;
        number_of_bytes = 32;
      }
      checksum_bits = 11 * seed_length - number_of_bits;
      oled.setCursor(0, PRIMER_RENGLON);
      if (seleccion == OPCION_SPLIT) oled.print("Split ");
      if (seleccion == OPCION_RECOVER) oled.print("Recover ");
      if (seed_length == 24) oled.print("24");
      else oled.print("12");
      for (int i = oled.getCursorX() >> 3; i < 16; i++) oled.write(' ');
      oled.setCursor(0, SEGUNDO_RENGLON);
      if (seleccion == OPCION_SPLIT) oled.print("Ingrese seed");
      if (seleccion == OPCION_RECOVER) oled.print("Ingrese 2 seeds");
      for (int i = oled.getCursorX() >> 3; i < 16; i++) oled.write(' ');
      oled.on();
      delay(2000);
      oled.setCursor(0, SEGUNDO_RENGLON);
      for (int i = 0; i < 16; i++) oled.write(' ');
      oled.on();
      menu = CARGA_PALABRA;
      op_cont = 0;
    } else if (button_state == LEFT || button_state == RIGHT) op_cont++;
    if (menu == O12O24) {
      oled.setCursor(0, PRIMER_RENGLON);
      oled.print("Largo de Seed:  ");
      oled.setCursor(0, SEGUNDO_RENGLON);
      if (op_cont % 2 == 0) oled.print("  [12]    24    ");
      else oled.print("   12    [24]   ");
      oled.on();
    }
  }

  if (menu == SHOW_SHARES && button_state != old_button_state) {
    old_button_state = button_state;
    if (button_state == LEFT)
      if (op_cont < 3 * seed_length - 1) op_cont++;
    if (button_state == RIGHT)
      if (op_cont > 0) op_cont--;
    if (menu == SHOW_SHARES) {
      byte s = op_cont / seed_length;
      byte w = op_cont % seed_length;
      oled.setCursor(0, PRIMER_RENGLON);
      oled.print("Share ");
      oled.print(s + 1);
      oled.print(", Word ");
      oled.print(w + 1);
      oled.write(' ');
      oled.setCursor(0, SEGUNDO_RENGLON);
      oled.print(word_list[share[s][w]]);
      for (int i = oled.getCursorX() >> 3; i < 16; i++) oled.write(' ');
      oled.on();
    }
  }

  if (menu == MENU_RECOVER && button_state != old_button_state) {
    old_button_state = button_state;
    if (button_state == LEFT)
      if (op_cont < seed_length - 1) op_cont++;
    if (button_state == RIGHT)
      if (op_cont > 0) op_cont--;
    if (menu == MENU_RECOVER) {
      oled.setCursor(0, PRIMER_RENGLON);
      oled.print("Word number ");
      oled.print(op_cont + 1);
      for (int i = oled.getCursorX() >> 3; i < 16; i++) oled.write(' ');
      oled.setCursor(0, SEGUNDO_RENGLON);
      oled.print(word_list[seed_phrase[op_cont]]);
      for (int i = oled.getCursorX() >> 3; i < 16; i++) oled.write(' ');
      oled.on();
    }
  }

  if (menu == CARGA_PALABRA && button_state != old_button_state) {
    old_button_state = button_state;

    oled.setCursor(0, PRIMER_RENGLON);
    if (seleccion == OPCION_SPLIT) oled.print("Word number ");
    if (seleccion == OPCION_RECOVER) {
      if (seeds_cargadas == 0) oled.print("Seed 1, Word ");
      else oled.print("Seed 2, Word ");
    }
    oled.print(word_cont + 1);
    for (int i = oled.getCursorX() >> 3; i < 16; i++) oled.write(' ');
    oled.on();

    if (button_state == LEFT) {
      letter_cont++;
      if (letter_cont == LETRA_BORRA + 1) letter_cont = 0;
      pone_siguiente_letra(1);

    } else if (button_state == RIGHT) {
      if (letter_cont == 0) letter_cont = LETRA_BORRA + 1;
      letter_cont--;
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
        } else {
          if (word_cont > 0) {
            word_cont--;
            op_cont = 3;
            const char* w = word_list[seed_phrase[word_cont]];
            palabra[0] = w[0] - 'a';
            palabra[1] = w[1] - 'a';
            palabra[2] = w[2] - 'a';
            if (w[3] == 0) palabra[3] = LETRA_ESPACIO;
            else palabra[3] = w[3] - 'a';

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
        int pos_word = busca_palabra();

        oled.setCursor(0, SEGUNDO_RENGLON);
        oled.print(word_list[pos_word]);
        oled.print('.');
        oled.on();
        seed_phrase[word_cont] = pos_word;
        word_cont++;
        op_cont = 0;
        letter_cont = 0;
        delay(1000);
        oled.setCursor(0, SEGUNDO_RENGLON);
        for (byte i = 0; i < 16; i++) oled.write(' ');
        oled.on();
        memset(palabra, 0xff, 4);
      }

      if (word_cont == seed_length) {  
        if (seleccion == OPCION_SPLIT) {
          split();
          menu = SHOW_SHARES;
        }
        if (seleccion == OPCION_RECOVER) {
          memcpy(share[seeds_cargadas], seed_phrase, sizeof(share[seeds_cargadas]));
          seeds_cargadas++;
          if (seeds_cargadas == 2) {
            recover();
            menu = MENU_RECOVER;
          }
        }
        op_cont = 0;
        word_cont = 0;
        letter_cont = 0;
      }
    }

    if (menu == CARGA_PALABRA) {
      oled.setCursor(op_cont * LARGO_LETRA, SEGUNDO_RENGLON);
      if (letter_cont == LETRA_BORRA) oled.write('<');
      else if (letter_cont == LETRA_ESPACIO) oled.write(' ');
      else oled.write('a' + letter_cont);
      oled.on();
    }
  }
}

void split() {
  byte seed_bytes[number_of_bytes];
  byte share_bytes[3][number_of_bytes];
  byte cs0, cs1, cs2;
  seed_to_bytes(seed_phrase, seed_bytes);

  byte random_values[number_of_bytes];
  memcpy(random_values, seed_bytes, sizeof(random_values));

  while (true) {
    sha256.reset();
    sha256.update(random_values, sizeof(random_values));
    sha256.finalize(random_values, sizeof(random_values));

    for (int i = 0; i < number_of_bytes; i++) {
      byte m = random_values[i];
      if (m == 0) m = 0xf;
      share_bytes[0][i] = gadd(gmul(m, 1), seed_bytes[i]);
      share_bytes[1][i] = gadd(gmul(m, 2), seed_bytes[i]);
      share_bytes[2][i] = gadd(gmul(m, 3), seed_bytes[i]);
    }

    sha256.reset();
    sha256.update(share_bytes[0], sizeof(share_bytes[0]));
    sha256.finalize(&cs0, 1);
    sha256.reset();
    sha256.update(share_bytes[1], sizeof(share_bytes[1]));
    sha256.finalize(&cs1, 1);
    sha256.reset();
    sha256.update(share_bytes[2], sizeof(share_bytes[2]));
    sha256.finalize(&cs2, 1);
    cs0 >>= 8 - checksum_bits;
    cs1 >>= 8 - checksum_bits;
    cs2 >>= 8 - checksum_bits;
    if (cs0 % 4 == 1 && cs1 % 4 == 2 && cs2 % 4 == 3) break;
  }
  bytes_to_seed(share_bytes[0], share[0], cs0);
  bytes_to_seed(share_bytes[1], share[1], cs1);
  bytes_to_seed(share_bytes[2], share[2], cs2);
}

void recover() {
  static const byte v_table[4][4] = {
    {0,   0,   0,   0}, 
    {0,   0, 247, 140}, 
    {0, 246,   0,   3}, 
    {0, 141,   2,   0}  
  };
  byte x0 = share[0][seed_length - 1] % 4;
  byte x1 = share[1][seed_length - 1] % 4;
  byte v0 = v_table[x0][x1];
  byte v1 = v_table[x1][x0];
  
  byte share_bytes[2][number_of_bytes];
  byte seed_bytes[number_of_bytes];
  byte cs;
  seed_to_bytes(share[0], share_bytes[0]);
  seed_to_bytes(share[1], share_bytes[1]);
  for (int i = 0; i < number_of_bytes; i++)
    seed_bytes[i] = gadd(gmul(share_bytes[0][i], v0), gmul(share_bytes[1][i], v1));
  sha256.reset();
  sha256.update(seed_bytes, sizeof(seed_bytes));
  sha256.finalize(&cs, 1);
  cs >>= 8 - checksum_bits;
  bytes_to_seed(seed_bytes, seed_phrase, cs);
}
