# 🌱 SeedSplitter
### Protegiendo Soberanías

SeedSplitter es un dispositivo de hardware que divide tu seed phrase de Bitcoin en 3 partes distintas, cada una una wallet BIP39 válida y funcional.

---

## ¿Cómo funciona?

- Ingresás tu seed phrase usando los 3 botones del dispositivo
- El dispositivo genera 3 seed phrases válidas y distintas
- Necesitás al menos 2 de las 3 para reconstruir la original
- Todo ocurre offline, sin internet, sin servidor, sin memoria persistente

---

## Recuperación sin el dispositivo

Si perdés el hardware, podés reconstruir tu seed original usando `recover.py` y cualquier computadora.

```bash
python recover.py
```

El script te va a pedir las 2 partes y devuelve la seed original. No requiere conexión a internet.

---

## Seguridad

- Sin WiFi ni Bluetooth — no transmite datos
- Sin memoria persistente — al apagar, no queda ningún rastro de la seed
- Hardware 100% visible — podés auditar cada componente
- Código abierto — podés auditar este repositorio

---

## Sitio web

[seedsplitter.com.ar](https://seedsplitter.com.ar)

---

*Hecho en Argentina 🇦🇷*
