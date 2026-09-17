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

---

## Tests reproducibles

El repositorio incluye pruebas reproducibles para verificar `Split`, `Generate` y la exportación watch-only en 12 y 24 palabras.

- [`TEST.md`](TEST.md): explica cómo ejecutar las pruebas y comparar los resultados con el hardware.
- [`test.py`](test.py): genera casos de prueba nuevos para `Split` y `Generate`, incluida la `zpub` y las primeras direcciones BIP84.

```bash
python3 test.py
```

Las seeds y tiradas generadas durante estas pruebas son sólo para testeo y **nunca deben usarse para guardar fondos**.

---

## Recuperación sin el dispositivo

Si perdés el hardware, podés reconstruir tu seed original usando `recover.py` y cualquier computadora.

```bash
python recover.py
```

El script te va a pedir las 2 partes y devuelve la seed original. No requiere conexión a internet.

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
