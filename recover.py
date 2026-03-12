import galois
from mnemonic import Mnemonic


def recover_seed(share0_words, share1_words):
    """
    Recupera la seed phrase original a partir de dos shares BIP39.

    Parámetros
    ----------
    share0_words : lista de 12/24 palabras del primer share
    share1_words : lista de 12/24 palabras del segundo share

    Retorna
    -------
    Lista de 12/24 palabras de la seed recuperada
    """
    
    # Aritmética en GF(2^8) con polinomio irreducible de Rijndael (0x11b)
    GF256 = galois.GF(2**8, irreducible_poly=0x11b)

    mnemo = Mnemonic("english")

    x0 = GF256(mnemo.wordlist.index(share0_words[-1]) % 4)
    x1 = GF256(mnemo.wordlist.index(share1_words[-1]) % 4)

    if x0 == x1 or x0 == 0 or x1 == 0:
        raise ValueError(
            f"Par de shares inválido: x0={x0}, x1={x1}. "
            "Asegurate de que los shares tengan x distintos."
        )

    # Coeficientes de Lagrange para recuperar f(0) dados dos puntos
    v0 = x1 / (x1 - x0)
    v1 = x0 / (x0 - x1)

    sb0 = mnemo.to_entropy(" ".join(share0_words))
    sb1 = mnemo.to_entropy(" ".join(share1_words))

    # f(0) = v0·y0 + v1·y1  byte a byte en GF(2^8)
    num_bytes = 32
    if WORDS == 12:
        num_bytes = 16
    recovered = bytes(
        int(GF256(sb0[i]) * v0 + GF256(sb1[i]) * v1)
        for i in range(num_bytes)
    )

    return mnemo.to_mnemonic(recovered).split()


# ---------------------------------------


WORDS = 24 # o 12
if WORDS != 24 and WORDS != 12:
	print("Error: deben ser 12 o 24 palabras")
	exit()
	
# Reemplazá estas listas con tus shares reales
share0 = input(f"Ingresá las {WORDS} palabras del share 1 (separadas por espacio):\n").split()
share1 = input(f"Ingresá las {WORDS} palabras del share 2 (separadas por espacio):\n").split()

if len(share0) != WORDS or len(share1) != WORDS:
	print(f"Error: cada share debe tener exactamente {WORDS} palabras.")
else:
	try:
		result = recover_seed(share0, share1)
		print("\nSeed phrase recuperada:")
		print(" ".join(result))
	except Exception as e:
		print(f"Error: {e}")
