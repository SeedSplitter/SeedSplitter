# SeedSplitter (STM32/BluePill)

**SeedSplitter** es una herramienta de seguridad diseñada para dividir una frase semilla (Mnemonic Seed) de 12 o 24 palabras en tres fragmentos (shares) utilizando un esquema de secreto compartido. Para recuperar la semilla original, solo se necesitan 2 de los 3 fragmentos generados.

Este proyecto está optimizado para funcionar en una **STM32F103C8 (BluePill)** con una pantalla OLED de 128x32.

## 🛠 Configuración del Entorno (Arduino IDE)

Para compilar, configura el Arduino IDE con los siguientes parámetros:

1.  **Placa:** `Generic STM32F1 series`
2.  **Board part number:** `BluePill F103C8`
3.  **Upload Method:** `STM32CubeProgrammer (SWD)`
4.  **USB Support:** `CDC (generic Serial supersede U(S)ART)`
5.  **Librerías necesarias:**
    * `Tiny4kOLED`: Para el manejo de la pantalla SSD1306.
    * `Crypto`: SHA-256 y SHA-512/HMAC para BIP-39/BIP-32.
    * `micro-ecc`: Para la multiplicación de clave pública `secp256k1` usada al construir la `zpub`.

**Configuración IDE:** Exceptuando los parámetros mencionados arriba, el resto de las configuraciones del Arduino IDE pueden dejarse por defecto.


## 🔌 Conexiones (Pinout)

### Cargar el software (ST-Link a BluePill)

Para cargar el codigo compilado utilizar un `ST-Link` conectado como se detalla

| ST-Link | BluePill |
| :--- | :--- |
| 3.3V | 3V3 |
| SWDIO | SWO (DIO) |
| SWDCLK | SWCLK |
| GND | GND |

### Pantalla OLED
| OLED | BluePill |
| :--- | :--- |
| SDA | PB7 |
| SCL | PB6 |

### Botones de Navegación
* **ENTER:** PA0
* **DERECHA:** PA1
* **IZQUIERDA:** PA2

## 📋 Ejemplos de Testeo

Puedes verificar el funcionamiento del algoritmo utilizando los casos de prueba incluidos en el código:

### Test 1: Semilla de 12 palabras
**Seed:** `abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about`

**Fragmentos resultantes:**

1. `blood dinner alcohol happy duty student bird repair design ripple endless certain`
2. `conduct proof slot spike matrix cat earn common issue prosper narrow layer`
3. `decrease must skate rice soap visa fatigue ocean estate bitter survey foot`

### Test 2: Semilla de 24 palabras
**Seed:** `cook wire acid abuse truly evoke super head insect wedding lonely orchard nuclear damp song winter gossip blue vacant midnight order art glide mystery`

**Fragmentos resultantes:**

1. `excuse question property more proud scrub recall rocket true peanut lemon tragic fall mixed tenant chief dutch predict mother prosper spatial another doll pass`
2. `useful local bright brother ginger extra good merry final enlist digital garment buffalo early urban happy cave phone involve soap below man equip rough`
3. `monster special ranch must chicken tell attitude food skull because screen celery sugar wire youth rebel affair bar biology unhappy flash logic hurdle movie`

## 🚀 Uso

Al encender el dispositivo, el menú principal muestra las tres opciones disponibles: **Split**, **Recover** y **Generate**. Usa los botones **LEFT** y **RIGHT** para mover la selección y **ENTER** para confirmar.

Después de elegir una opción, selecciona si trabajarás con una frase de **12** o **24 palabras**.

### Split

Usa **Split** para dividir una seed existente en tres partes.

1. Selecciona **Split** y luego **12** o **24 palabras**.
2. Introduce la seed palabra por palabra. Para cada palabra, navega por las letras con **LEFT/RIGHT** y confirma con **ENTER**. El sistema filtra automáticamente las letras válidas según el diccionario BIP-39.
3. Al terminar, SeedSplitter genera **3 partes**.
4. Anota y guarda las tres partes por separado. **Cualquier combinación de 2 partes permite recuperar la seed original.**

### Recover

Usa **Recover** para reconstruir una seed a partir de dos de las tres partes generadas previamente.

1. Selecciona **Recover** y luego **12** o **24 palabras**.
2. Introduce la primera parte palabra por palabra.
3. Introduce la segunda parte de la misma manera.
4. SeedSplitter reconstruye y muestra la seed original.

### Generate

Usa **Generate** para crear una nueva seed a partir de **50 tiradas de un dado físico de seis caras**.

1. Selecciona **Generate** y luego **12** o **24 palabras**.
2. Tira un dado físico 50 veces. Para cada tirada, selecciona el valor **1–6** con **LEFT/RIGHT** y confirma con **ENTER**; usa `<` si necesitas corregir la tirada anterior.
3. SeedSplitter genera y muestra la frase BIP-39. Usa **LEFT** para avanzar por las palabras y **RIGHT** para volver atrás. Anota la frase y guárdala de forma segura.
4. Desde la última palabra, pulsa **LEFT** para acceder a la exportación watch-only por `zpub`. Pulsa **RIGHT** desde el QR para volver a las palabras. La `zpub` es pública y regenerable; la frase BIP-39 es el backup importante y no debe compartirse.

### Export watch-only

El QR está adaptado específicamente a la pantalla OLED de **128x32**. Se usan QR Version 1-L de 21x21 módulos con una zona blanca de 4 píxeles; el símbolo completo ocupa 29x29 píxeles y se coloca en el extremo izquierdo. Los dos renglones disponibles a la derecha muestran `Watch-only` y `zpub nn/NN`. Una `zpub` normal de 111 caracteres se divide en 12 frames BBQr Base32 y cada frame cambia aproximadamente cada 300 ms.

La derivación usa passphrase BIP-39 vacía y Bitcoin mainnet:

`BIP39 -> seed (PBKDF2-HMAC-SHA512) -> BIP32 -> m/84'/0'/0' -> zpub`

El material privado intermedio se mantiene únicamente en RAM y se sobrescribe después de construir la `zpub`.


---

### ⚠️ Notas de Seguridad
* **Entorno Offline:** Este software maneja claves privadas críticas. Se recomienda estrictamente ejecutarlo en hardware que nunca haya sido (ni sea) conectado a internet.
* **Flash:** la Blue Pill probada reporta 128 KB físicos, pero el objetivo sigue siendo permanecer dentro del límite oficial/configurado de 64 KB.
* **Prueba antes de usar fondos:** verifica primero el firmware con vectores conocidos y con una wallet sin fondos. El archivo `WATCHONLY_TEST.md` contiene un vector oficial BIP-84.


