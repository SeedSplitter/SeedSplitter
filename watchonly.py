## Genera una seed phase segura y una billetera watch-only. 
## Se recomienda ejecutar el script en un Live OS (como Tails o Ubuntu) 
## desde un USB, en una PC air-gapped (sin conexión a internet).

import os
import mnemonic
import qrcode
from bip_utils import Bip39SeedGenerator, Bip84, Bip84Coins, Bip39WordsNum

num_palabras = 12 # o 24

# 1. Generar entropía como Satoshi desde el SO (/dev/urandom o Windows Crypto API)
# 12 palabras = 128 bits | 24 palabras = 256 bits
strength = 128 if num_palabras == 12 else 256
entropy = os.urandom(strength // 8)

# 2. Convertir entropía a palabras (Mnemonic)
mnemo = mnemonic.Mnemonic("english")
seed_phrase = mnemo.to_mnemonic(entropy)
print(f"--- seed phrase ---")
print(seed_phrase)

# 3. Generar Seed Binaria a partir de las palabras
seed_bytes = Bip39SeedGenerator(seed_phrase).Generate()

# 4. Derivar Master Key para SegWit Nativo (BIP84)
bip84_mst = Bip84.FromSeed(seed_bytes, Bip84Coins.BITCOIN)

# 5. Obtener la Cuenta 0 (m/84'/0'/0')
# Esta es la que se usa para "Watch-Only"
bip84_acc = bip84_mst.Purpose().Coin().Account(0)
zpub = bip84_acc.PublicKey().ToExtended()
	
# 6. Generar Código QR
img = qrcode.make(zpub)
img.save("zpub_qr.png")
print("--- zpub guardada como QR en 'zpub_qr.png' ---")
print(zpub)

