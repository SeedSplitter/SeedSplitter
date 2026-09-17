# SeedSplitter — Hardware Test

`test.py` genera cuatro pruebas para comparar directamente con un SeedSplitter.

El objetivo no es comprobar matemáticamente el algoritmo, sino verificar que el
dispositivo produce exactamente los mismos resultados que la implementación
Python.

## Requisitos

```bash
pip install mnemonic pyfinite bip-utils
```

## Ejecutar los tests

Desde la raíz del repositorio:

```bash
python3 test.py
```

El script genera, en este orden:

1. **Split — 12 palabras**
2. **Split — 24 palabras**
3. **Generate — 12 palabras**
4. **Generate — 24 palabras**

En cada prueba se muestra:

- qué opción seleccionar en SeedSplitter;
- qué seed o tiradas ingresar;
- qué resultado debería mostrar exactamente el dispositivo.

En los tests de **Generate**, además de la frase BIP39, `test.py` calcula la
`zpub` BIP84 y las primeras tres direcciones de recepción `bc1q...`. Después de
escanear el QR watch-only con una wallet compatible, por ejemplo BlueWallet,
esas tres direcciones deben coincidir exactamente.

Compará el resultado del hardware palabra por palabra con el resultado mostrado
por `test.py`.

Cada ejecución genera ejemplos nuevos.

## Qué se está comprobando

Para los tests de **Split**, el script genera una seed BIP39 aleatoria y calcula
las tres partes que SeedSplitter debería mostrar.

Para los tests de **Generate**, genera 50 tiradas de dado de prueba, calcula la
seed BIP39 que SeedSplitter debería mostrar y deriva de manera independiente la
cuenta BIP84 `m/84'/0'/0'`, su `zpub` y las primeras tres direcciones externas
`m/84'/0'/0'/0/0`, `/0/1` y `/0/2`.

Para comprobar **Recover**, pueden ingresarse dos de las tres shares producidas
en un test de Split: la seed recuperada debe coincidir con la seed original de
ese mismo test.

De esta forma, Generate, Split, Recover y la exportación watch-only pueden
compararse con una implementación independiente. El usuario puede verificar el
comportamiento del dispositivo sin tener que confiar en que sus resultados sean
correctos por sí solos.

## Seguridad

Las seeds y tiradas producidas por `test.py` son solamente para pruebas.

**No uses ninguna seed generada durante estos tests para guardar fondos.**
