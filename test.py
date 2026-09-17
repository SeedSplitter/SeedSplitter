import hashlib
import secrets
from mnemonic import Mnemonic
from pyfinite import ffield
F = ffield.FField(8, gen=283, useLUT=0)
mnemo = Mnemonic("english")

def split(seed_words):
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



print("\nSeedSplitter — Test")
print("-------------------")
print("Compará cada resultado con el mostrado por el dispositivo.")
print("No uses ninguna seed de este test para guardar fondos.")

for number, words in enumerate((12, 24), 1):
    seed = mnemo.to_mnemonic(secrets.token_bytes(16 if words == 12 else 32))
    shares = split(seed)

    print("\n" + "=" * 72)
    print(f"TEST {number} — SPLIT — {words} PALABRAS")
    print("=" * 72)
    print(f"\nEn SeedSplitter elegí:\n  Split -> {words} words")
    print(f"\nIngresá esta seed:\n\n{seed}")
    print("\nEl dispositivo debe mostrar exactamente:")

    for i, share in enumerate(shares, 1):
        print(f"\nSHARE {i}\n{share}")


for number, words in enumerate((12, 24), 3):
    rolls = "".join(secrets.choice("123456") for _ in range(50))
    digest = hashlib.sha256(rolls.encode("ascii")).digest()
    seed = mnemo.to_mnemonic(digest[:16] if words == 12 else digest)

    print("\n" + "=" * 72)
    print(f"TEST {number} — GENERATE — {words} PALABRAS")
    print("=" * 72)
    print(f"\nEn SeedSplitter elegí:\n  Generate -> {words} words")
    print(f"\nIngresá estas 50 tiradas, en este orden:\n\n{rolls}")
    print(f"\nEl dispositivo debe mostrar exactamente esta seed:\n\n{seed}")

print("\n" + "=" * 72)
