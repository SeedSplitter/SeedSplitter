## Genera la exportación watch-only (zpub) de una seed BIP39 existente.
## Se recomienda ejecutar el script en un Live OS (como Tails o Ubuntu)
## desde un USB, en una PC air-gapped (sin conexión a internet).

import mnemonic
import qrcode
from bip_utils import Bip39SeedGenerator, Bip84, Bip84Coins


mnemo = mnemonic.Mnemonic("english")
seed_phrase = " ".join(
    input("Ingresá la seed phrase de 12 o 24 palabras:\n").strip().split()
)
words = seed_phrase.split()

if len(words) not in (12, 24):
    raise ValueError("La seed debe tener 12 o 24 palabras.")
if not mnemo.check(seed_phrase):
    raise ValueError("Seed BIP39 inválida. Revisá las palabras y el checksum.")

# Derivar la cuenta BIP84 m/84'/0'/0' para importarla como watch-only.
seed_bytes = Bip39SeedGenerator(seed_phrase).Generate()
bip84_mst = Bip84.FromSeed(seed_bytes, Bip84Coins.BITCOIN)
bip84_acc = bip84_mst.Purpose().Coin().Account(0)
zpub = bip84_acc.PublicKey().ToExtended()

img = qrcode.make(zpub)
img.save("zpub_qr.png")

print("\n--- zpub / watch-only ---")
print(zpub)
print("\nQR guardado como 'zpub_qr.png'.")
