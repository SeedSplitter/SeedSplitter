# Test de watch-only BIP-84

Antes de usar SeedSplitter con fondos reales, conviene verificar la derivación con el vector oficial de BIP-84.

## Vector

Mnemonic:

```text
abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about
```

Derivación de cuenta:

```text
m/84'/0'/0'
```

`zpub` esperada:

```text
zpub6rFR7y4Q2AijBEqTUquhVz398htDFrtymD9xYYfG1m4wAcvPhXNfE3EfH1r1ADqtfSdVCToUG868RvUUkgDKf31mGDtKsAYz2oz2AGutZYs
```

La implementación de `watchonly.cpp` fue comparada contra este vector.

## BBQr

Para esa zpub la implementación genera 12 frames BBQr (`B$2U...`). El encoder QR fijo Version 1-L fue contrastado módulo a módulo contra una implementación QR de referencia y todos los frames fueron decodificados nuevamente durante las pruebas de desarrollo.

## Prueba práctica sugerida

1. Compila el sketch y revisa Flash/RAM.
2. Usa primero una seed de prueba sin fondos.
3. Escanea el BBQr desde **Add wallet -> Import wallet -> Scan** en BlueWallet.
4. Comprueba que la wallet resultante es watch-only y que la `zpub`/primeras direcciones coinciden con otra implementación BIP-84 independiente.
5. Sólo después usa el flujo con tiradas reales.
