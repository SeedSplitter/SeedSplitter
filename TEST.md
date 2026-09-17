# SeedSplitter — Hardware Test

`test.py` genera cuatro pruebas para comparar directamente con un SeedSplitter.

El objetivo no es comprobar matemáticamente el algoritmo, sino verificar que el
dispositivo produce exactamente los mismos resultados que la implementación
Python.

## Requisitos

```bash
pip install mnemonic pyfinite
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

Compará el resultado del hardware palabra por palabra con el resultado mostrado
por `test.py`.

Cada ejecución genera ejemplos nuevos.

## Qué se está comprobando

Para los tests de **Split**, el script genera una seed BIP39 aleatoria y calcula
las tres partes que SeedSplitter debería mostrar.

Para los tests de **Generate**, genera 50 tiradas de dado de prueba y calcula la
seed BIP39 que SeedSplitter debería mostrar.

Si los resultados coinciden, el dispositivo está reproduciendo los mismos
algoritmos que la implementación Python.

## Seguridad

Las seeds y tiradas producidas por `test.py` son solamente para pruebas.

**No uses ninguna seed generada durante estos tests para guardar fondos.**
