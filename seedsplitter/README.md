# SeedSplitter (STM32/BluePill)

**SeedSplitter** es una herramienta de seguridad diseñada para dividir una frase semilla (Mnemonic Seed) de 12 o 24 palabras en tres fragmentos (shares) utilizando un esquema de secreto compartido. Para recuperar la semilla original, solo se necesitan 2 de los 3 fragmentos generados.

Este proyecto está optimizado para funcionar en una **STM32F103C8 (BluePill)** con una pantalla OLED de 128x32.

## 🛠 Configuración del Entorno (Arduino IDE)

Para compilar, configura el Arduino IDE con los siguientes parámetros:

1.  **Placa:** `Generic STM32F1 series`
2.  **Board part number:** `BluePill F103C8`
3.  **Upload Method:** `STM32CubeProgrammer (SWD)`
4.  **USB Support:** `None`
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
