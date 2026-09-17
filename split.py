## Divide una seed phrase en 3 partes, de las cuales cualquier par permite recuperarla.
## Se recomienda ejecutar el script en un Live OS (como Tails o Ubuntu)
## desde un USB, en una PC air-gapped (sin conexión a internet).

import hashlib
from mnemonic import Mnemonic
from pyfinite import ffield

F = ffield.FField(8, gen=283, useLUT=0)
mnemo = Mnemonic("english")


def split(seed_words):
    words = seed_words.split()
    if len(words) not in (12, 24):
        raise ValueError("La seed debe tener 12 o 24 palabras.")
    if not mnemo.check(seed_words):
        raise ValueError("Seed BIP39 inválida. Revisá las palabras y el checksum.")

    seed = bytes(mnemo.to_entropy(seed_words))
    random_values = seed
    checksum_bits = 4 if len(seed) == 16 else 8

    while True:
        random_values = hashlib.sha256(random_values).digest()[:len(seed)]
        shares, checksums = [], []

        for x in (1, 2, 3):
            share = bytes(
                F.Add(F.Multiply(m or 0x0F, x), secret)
                for m, secret in zip(random_values, seed)
            )
            shares.append(share)
            checksums.append(
                hashlib.sha256(share).digest()[0] >> (8 - checksum_bits)
            )

        if [c % 4 for c in checksums] == [1, 2, 3]:
            return [mnemo.to_mnemonic(share) for share in shares]


seed_words = " ".join(input("Ingresá la seed phrase de 12 o 24 palabras:\n").strip().split())
shares = split(seed_words)

for i, share in enumerate(shares, 1):
    print(f"\n--- share {i} ---")
    print(share)
