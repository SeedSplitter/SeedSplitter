# 🌱 SeedSplitter

### Protegiendo Soberanías

SeedSplitter es un dispositivo de hardware que divide tu seed phrase de Bitcoin en 3 partes distintas, cada una una wallet BIP39 válida y funcional.

También puede generar una seed BIP39 nueva a partir de entropía física aportada por el usuario: 50 tiradas independientes de un dado de seis caras.

---

## ¿Cómo funciona?

### Dividir una seed existente

- Ingresás tu seed phrase usando los 3 botones del dispositivo.
- SeedSplitter genera 3 seed phrases BIP39 válidas y distintas.
- Guardás las tres partes en ubicaciones diferentes.
- Necesitás al menos 2 de las 3 para reconstruir la original.
- Todo ocurre offline, sin internet ni servidor.

### Recuperar una seed

- Ingresás 2 de las 3 partes.
- SeedSplitter reconstruye la seed original.
- También podés recuperar sin el dispositivo usando `recover.py`.

### Generar tu propia seed

Si querés crear una seed desde cero:

1. Elegí **Generate** en el menú.
2. Elegí **12** o **24 palabras**.
3. Tirás un dado físico de seis caras **50 veces** e ingresás cada resultado (`1` a `6`).
4. SeedSplitter genera una frase BIP39 válida completamente offline.

Las 50 tiradas están pensadas para alcanzar **~128 bits de seguridad criptográfica**, en línea con el nivel de seguridad clásico de las claves secp256k1 usadas por Bitcoin. Siempre deben ser independientes, secretas y realizadas con un dado razonablemente justo.

### Generar la zpub / watch-only

Para obtener la información pública de una seed sin cargarla en una computadora:

1. Elegí **zpub** en el menú.
2. Elegí **12** o **24 palabras**.
3. Ingresá la seed manualmente.
4. SeedSplitter valida el checksum BIP39 y, si es correcto, genera la `zpub` BIP84 y la muestra como QR animado.

Este flujo también sirve para volver a ingresar una seed recién creada con **Generate** y verificar que fue anotada correctamente antes de fondearla.

---

## Tests reproducibles

El repositorio incluye `test.py` para comparar directamente el comportamiento del hardware con una implementación Python independiente.

Para ejecutarlo:

```bash
pip install mnemonic pyfinite bip-utils
python3 test.py
```

Cada ejecución genera cuatro casos nuevos, en este orden:

1. **Split — 12 palabras**
2. **Split — 24 palabras**
3. **Generate — 12 palabras**
4. **Generate — 24 palabras**

Las seeds y tiradas producidas por `test.py` son exclusivamente para pruebas y **nunca deben usarse para guardar fondos**.

---

## Scripts offline

El repositorio incluye cuatro scripts para usar las funciones principales desde una computadora. Sus dependencias pueden instalarse con:

```bash
pip install mnemonic pyfinite bip-utils "qrcode[pil]"
```

- `split.py`: divide una seed BIP39 de 12 o 24 palabras en 3 shares compatibles con SeedSplitter.
- `recover.py`: reconstruye la seed original a partir de 2 shares.
- `generate.py`: genera una seed de 12 o 24 palabras, usando 50 tiradas de un dado físico o el generador seguro del sistema operativo.
- `zpub.py`: recibe una seed BIP39 de 12 o 24 palabras, valida su checksum y exporta la cuenta BIP84 watch-only como `zpub` y QR.

```bash
python3 split.py
python3 recover.py
python3 generate.py
python3 zpub.py
```

Para trabajar con una seed real, se recomienda ejecutar estos scripts en un Live OS (como Tails o Ubuntu), desde un USB y en una PC air-gapped, sin conexión a internet.

---

## Seguridad

- Sin WiFi ni Bluetooth — no transmite datos.
- Sin memoria persistente — al apagar, no queda ningún rastro de la seed.
- Hardware 100% visible — podés auditar cada componente.
- Código abierto — podés auditar este repositorio.
- La generación de una seed nueva no depende de un generador aleatorio interno: la entropía proviene de las tiradas físicas del usuario.
- Para generar una seed real, usá un dado físico, mantené las 50 tiradas en secreto y no reutilices la misma secuencia.

---

## Sitio web

https://seedsplitter.com.ar

---

Hecho en Argentina 🇦🇷
