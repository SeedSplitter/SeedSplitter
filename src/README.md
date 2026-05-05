# SeedSplitter (STM32/BluePill)

**SeedSplitter** es una herramienta de seguridad diseñada para dividir una frase semilla (Mnemonic Seed) de 12 o 24 palabras en tres fragmentos (shares) utilizando un esquema de secreto compartido. Para recuperar la semilla original, solo se necesitan 2 de los 3 fragmentos generados.

Este proyecto está optimizado para funcionar en una **STM32F103C8 (BluePill)** con una pantalla OLED de 128x32.

## 🛠 Configuración del Entorno (Arduino IDE)

Para compilar y cargar este código, configura el Arduino IDE con los siguientes parámetros:

1.  **Placa:** `Generic STM32F1 series`
2.  **Board part number:** `BluePill F103C8`
3.  **Upload Method:** `STM32CubeProgrammer (SWD)` o `ST-Link`
4.  **USB Support:** `None` o `CDC (generic Serial superimpose UART)`
5.  **Librerías necesarias:**
    * `Tiny4kOLED`: Para el manejo de la pantalla SSD1306.
    * `sha256`: Para la generación de checksums y entropía.

## 🔌 Conexiones (Pinout)

### Programación (ST-Link V2 a BluePill)
| ST-Link V2 | BluePill |
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

## 🧪 Ejemplos de Testeo

Puedes verificar el funcionamiento del algoritmo utilizando los casos de prueba incluidos en el código:

### Test 1: Semilla de 12 palabras
* **Seed:** `abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about`
* **Fragmentos resultantes:** 1. `blood dinner alcohol happy duty student bird repair design ripple endless certain`
  2. `conduct proof slot spike matrix cat earn common issue prosper narrow layer`
  3. `decrease must skate rice soap visa fatigue ocean estate bitter survey foot`

### Test 2: Semilla de 24 palabras
* **Seed:** `cook wire acid abuse truly evoke super head insect wedding lonely orchard nuclear damp song winter gossip blue vacant midnight order art glide mystery`

## 🚀 Uso
1. **Inicio:** Al encender el dispositivo, selecciona entre **[Split]** (dividir una semilla propia) o **Recover** (recuperar una semilla desde fragmentos).
2. **Configuración:** Elige el largo de tu frase semilla: **12** o **24** palabras.
3. **Ingreso:** Introduce las palabras navegando por el abecedario. El sistema filtra automáticamente las letras válidas según el diccionario estándar BIP-39 para facilitar la carga.
4. **Resultado Split:** Si elegiste dividir, el dispositivo generará y mostrará los 3 shares. Debes anotar los tres.
5. **Resultado Recover:** Si elegiste recuperar, deberás ingresar 2 de los fragmentos previamente generados para reconstruir la semilla original.

---

### ⚠️ Notas de Seguridad
* **Entorno Offline:** Este software maneja claves privadas críticas. Se recomienda estrictamente ejecutarlo en hardware que nunca haya sido (ni sea) conectado a internet.
* **Configuración IDE:** Exceptuando los parámetros mencionados arriba, el resto de las configuraciones del Arduino IDE pueden dejarse por defecto.
