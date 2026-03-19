from pyfinite import ffield
from mnemonic import Mnemonic

share0_words = input(f"Ingresá las palabras del share 1 (separadas por espacio):\n")
share1_words = input(f"Ingresá las palabras del share 2 (separadas por espacio):\n")

F = ffield.FField(8, gen=283)
mnemo = Mnemonic("english")

sb0 = mnemo.to_entropy(share0_words)
sb1 = mnemo.to_entropy(share1_words)

x0 = mnemo.wordlist.index(share0_words.split()[-1]) % 4
x1 = mnemo.wordlist.index(share1_words.split()[-1]) % 4

if x0 == x1 or x0 == 0 or x1 == 0 or len(sb0) != len(sb1):
	raise ValueError("Par de shares inválido. Asegurate de que los shares tengan x distintos y misma cantidad de palabras.")

recovered = [F.Divide(F.Add(F.Multiply(s0, x1), F.Multiply(s1, x0)), F.Add(x0, x1)) for s0,s1 in zip(sb0,sb1)]
print(mnemo.to_mnemonic(bytes(recovered)))
