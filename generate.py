## Genera una seed phrase segura y una billetera watch-only.
## Se recomienda ejecutar el script en un Live OS (como Tails o Ubuntu)
## desde un USB, en una PC air-gapped (sin conexión a internet).

import hashlib
import os
import mnemonic
import qrcode
from bip_utils import Bip39SeedGenerator, Bip84, Bip84Coins


def read_word_count():
    value = input("Cantidad de palabras (12/24) [12]: ").strip() or "12"
    if value not in ("12", "24"):
        raise ValueError("La cantidad de palabras debe ser 12 o 24.")
    return int(value)


def read_entropy_source():
    print("\nFuente de entropía:")
    print("  1. 50 tiradas de un dado físico")
    print("  2. Generador seguro del sistema operativo")
    value = input("Elegí 1 o 2: ").strip()
    if value not in ("1", "2"):
        raise ValueError("Opción inválida. Elegí 1 o 2.")
    return value


def entropy_from_dice(num_words):
    rolls = "".join(input(
        "\nIngresá las 50 tiradas, en orden, usando sólo números del 1 al 6\n"
        "(podés escribirlas juntas o separadas por espacios):\n"
    ).split())

    if len(rolls) != 50 or any(c not in "123456" for c in rolls):
        raise ValueError("Debés ingresar exactamente 50 tiradas, todas entre 1 y 6.")

    digest = hashlib.sha256(rolls.encode("ascii")).digest()
    return digest[:16] if num_words == 12 else digest


def entropy_from_os(num_words):
    return os.urandom(16 if num_words == 12 else 32)


num_words = read_word_count()
source = read_entropy_source()
entropy = entropy_from_dice(num_words) if source == "1" else entropy_from_os(num_words)

mnemo = mnemonic.Mnemonic("english")
seed_phrase = mnemo.to_mnemonic(entropy)
print("\n--- seed phrase ---")
print(seed_phrase)

# Derivar la cuenta BIP84 m/84'/0'/0' para importarla como watch-only.
seed_bytes = Bip39SeedGenerator(seed_phrase).Generate()
bip84_mst = Bip84.FromSeed(seed_bytes, Bip84Coins.BITCOIN)
bip84_acc = bip84_mst.Purpose().Coin().Account(0)
zpub = bip84_acc.PublicKey().ToExtended()

img = qrcode.make(zpub)
img.save("zpub_qr.png")
print("\n--- zpub guardada como QR en 'zpub_qr.png' ---")
print(zpub)
