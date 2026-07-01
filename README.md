<div align="center">

# 🚗 AutoFull

### Robot Wi-Fi con Arduino UNO + ESP32, control web, sensores y personalidad OLED.

![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge\&logo=arduino\&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-20232A?style=for-the-badge\&logo=espressif\&logoColor=E7352C)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge\&logo=cplusplus\&logoColor=white)
![HTML](https://img.shields.io/badge/HTML5-E34F26?style=for-the-badge\&logo=html5\&logoColor=white)
![CSS](https://img.shields.io/badge/CSS3-1572B6?style=for-the-badge\&logo=css\&logoColor=white)
![JavaScript](https://img.shields.io/badge/JavaScript-F7DF1E?style=for-the-badge\&logo=javascript\&logoColor=111)

<br />

![GitHub repo size](https://img.shields.io/github/repo-size/Aznar-7/AutoFull?style=flat-square)
![GitHub last commit](https://img.shields.io/github/last-commit/Aznar-7/AutoFull?style=flat-square)
![GitHub stars](https://img.shields.io/github/stars/Aznar-7/AutoFull?style=flat-square)

</div>

---

## 📌 Descripción

**AutoFull** es un robot/autito Wi-Fi desarrollado con **Arduino UNO** y **ESP32**, controlado desde una interfaz web responsive alojada directamente en el ESP32.

El proyecto combina robótica, IoT y desarrollo web embebido: el **ESP32** se encarga de levantar un servidor web, gestionar la conexión Wi-Fi y enviar comandos por Serial al **Arduino UNO**, mientras que el Arduino controla los motores, sensores, pantalla OLED y lógica de evasión de obstáculos.

Además, el robot cuenta con una pantalla OLED con **ojos animados** que reaccionan al estado del auto, dándole una personalidad visual más expresiva.

---

## ✨ Características principales

* 📱 **Control web desde navegador o celular**
* 📡 **Conexión Wi-Fi mediante ESP32**
* 🧠 **Arduino UNO como controlador principal**
* 🕹️ **Movimiento hacia adelante, atrás, izquierda, derecha y stop**
* 🚧 **Detección y evasión de obstáculos**
* 👀 **Pantalla OLED con ojos animados**
* 🌡️ **Lectura de temperatura y humedad con DHT11**
* 📊 **Panel web con estado, sensores, temperatura y humedad**
* ⌨️ **Soporte para control por teclado**
* 👆 **Soporte para controles táctiles**
* 🔁 **Comunicación Serial entre ESP32 y Arduino UNO**
* 🛡️ **Watchdog en Arduino para mayor seguridad**

---

## 🧠 Arquitectura del sistema

```txt
┌─────────────────────┐
│   Interfaz Web       │
│ HTML + CSS + JS      │
│ Celular / Navegador  │
└──────────┬──────────┘
           │ HTTP
           ▼
┌─────────────────────┐
│        ESP32         │
│ WiFiManager          │
│ WebServer            │
│ LittleFS             │
│ mDNS: auto.local     │
└──────────┬──────────┘
           │ Serial2
           ▼
┌─────────────────────┐
│     Arduino UNO      │
│ Motores              │
│ Sensores             │
│ OLED                 │
│ DHT11                │
└──────────┬──────────┘
           ▼
┌─────────────────────┐
│      Robot Car       │
│ Movimiento + evasión │
└─────────────────────┘
```

---

## 📂 Estructura del proyecto

```txt
AutoFull/
├── AutoFull.ino          # Código principal del Arduino UNO
├── Esp32WifiCar.ino      # Código del ESP32: Wi-Fi, servidor web y puente Serial
└── data/
    ├── index.html        # Interfaz web del control remoto
    ├── script.js         # Lógica del frontend y comunicación con el ESP32
    └── style.css         # Estilos responsive de la interfaz
```

---

## 🔩 Hardware utilizado

| Componente                      | Función                                            |
| ------------------------------- | -------------------------------------------------- |
| **Arduino UNO**                 | Control principal del robot                        |
| **ESP32**                       | Servidor web, Wi-Fi y comunicación con la interfaz |
| **Motores DC**                  | Movimiento del auto                                |
| **Driver de motores**           | Control de dirección y velocidad                   |
| **Sensor ultrasónico**          | Detección frontal de obstáculos                    |
| **Sensores infrarrojos**        | Detección lateral / proximidad                     |
| **DHT11**                       | Temperatura y humedad                              |
| **Pantalla OLED SH1106 128x64** | Ojos animados del robot                            |
| **Batería / fuente externa**    | Alimentación del sistema                           |
| **Chasis de auto robot**        | Estructura física del proyecto                     |

---

## 🧰 Tecnologías y librerías

### Arduino UNO

* `IRremote`
* `U8g2`
* `DHT`
* `Wire`
* `avr/wdt`

### ESP32

* `WiFi`
* `WebServer`
* `WiFiManager`
* `ESPmDNS`
* `LittleFS`

### Frontend

* HTML5
* CSS3
* JavaScript Vanilla

---

## 🔌 Pines principales

### Arduino UNO

| Elemento            | Pin  |
| ------------------- | ---- |
| DHT11               | `2`  |
| Trigger ultrasónico | `7`  |
| Echo ultrasónico    | `4`  |
| Sensor IR izquierdo | `8`  |
| Sensor IR derecho   | `12` |
| Motor A1            | `10` |
| Motor A2            | `5`  |
| Motor B1            | `6`  |
| Motor B2            | `9`  |

### ESP32

| Comunicación | Pin  |
| ------------ | ---- |
| Serial2 RX   | `16` |
| Serial2 TX   | `17` |

---

## 📡 Comunicación entre módulos

El sistema usa comunicación Serial entre el **ESP32** y el **Arduino UNO**.

### Comandos enviados desde la web

| Acción    | Comando |
| --------- | ------- |
| Adelante  | `F`     |
| Atrás     | `B`     |
| Izquierda | `L`     |
| Derecha   | `R`     |
| Detener   | `S`     |

### Endpoints del ESP32

| Endpoint                | Método | Descripción                                      |
| ----------------------- | ------ | ------------------------------------------------ |
| `/`                     | `GET`  | Sirve la interfaz web                            |
| `/command?cmd=forward`  | `GET`  | Envía comando para avanzar                       |
| `/command?cmd=backward` | `GET`  | Envía comando para retroceder                    |
| `/command?cmd=left`     | `GET`  | Envía comando para girar a la izquierda          |
| `/command?cmd=right`    | `GET`  | Envía comando para girar a la derecha            |
| `/command?cmd=stop`     | `GET`  | Detiene el robot                                 |
| `/status`               | `GET`  | Devuelve el estado actual recibido desde Arduino |

---

## 👀 Sistema de ojos OLED

AutoFull incluye una animación de ojos estilo “robot companion” en una pantalla OLED SH1106.

El robot puede representar distintos estados visuales:

| Estado       | Comportamiento                            |
| ------------ | ----------------------------------------- |
| `HAPPY`      | Estado normal / reposo                    |
| `ANGRY`      | Obstáculo detectado                       |
| `TIRED`      | Movimiento hacia atrás                    |
| `SUSPICIOUS` | Estado reservado para futuras expresiones |

Funciones destacadas:

* Parpadeo aleatorio.
* Movimiento de pupilas.
* Expresiones según estado.
* Dibujo inmediato ante emergencias.
* Actualización fluida de pantalla.

---

## 🚧 Detección y evasión de obstáculos

El Arduino monitorea constantemente:

* Distancia frontal mediante sensor ultrasónico.
* Sensores IR laterales.
* Estado actual del robot.

Cuando detecta un obstáculo:

1. Cambia el estado a `OBSTACLE`.
2. Cambia la expresión OLED a enojo.
3. Detiene el robot.
4. Retrocede brevemente.
5. Gira hacia una dirección segura.
6. Vuelve al estado normal.

---

## 📱 Interfaz web

La interfaz web está alojada en el ESP32 usando **LittleFS**.

Incluye:

* Botones grandes de dirección.
* Botón de stop.
* Estado de conexión.
* Estado del robot.
* Lectura de sensores.
* Temperatura.
* Humedad.
* Indicadores visuales.
* Diseño responsive para celular.
* Soporte para teclado.

### Controles por teclado

| Tecla        | Acción    |
| ------------ | --------- |
| `ArrowUp`    | Adelante  |
| `ArrowDown`  | Atrás     |
| `ArrowLeft`  | Izquierda |
| `ArrowRight` | Derecha   |
| `Space`      | Stop      |

---

## 🚀 Instalación y carga

### 1. Clonar el repositorio

```bash
git clone https://github.com/Aznar-7/AutoFull.git
cd AutoFull
```

---

### 2. Cargar el código del Arduino UNO

Abrir `AutoFull.ino` en Arduino IDE.

Instalar las librerías necesarias:

```txt
IRremote
U8g2
DHT sensor library
Adafruit Unified Sensor
```

Seleccionar la placa:

```txt
Arduino UNO
```

Cargar el sketch al Arduino.

---

### 3. Cargar el código del ESP32

Abrir `Esp32WifiCar.ino` en Arduino IDE.

Instalar las librerías necesarias:

```txt
WiFiManager
ESP32 board package
```

Seleccionar la placa ESP32 correspondiente.

Cargar el sketch al ESP32.

---

### 4. Subir archivos web a LittleFS

La carpeta `data/` contiene los archivos de la interfaz web:

```txt
data/
├── index.html
├── script.js
└── style.css
```

Subir estos archivos al sistema de archivos del ESP32 usando LittleFS.

---

## 🌐 Uso

Al encender el ESP32, se crea un punto de acceso para configurar la red Wi-Fi:

```txt
SSID: AutoAp
Password: Auto2308
```

Una vez conectado a la red, el ESP32 inicia el servidor web.

Podés acceder desde el navegador usando la IP mostrada por Serial Monitor o mediante mDNS:

```txt
http://auto.local
```

Desde la interfaz web se puede controlar el robot, ver el estado de conexión, sensores, temperatura y humedad.

---

## 🖼️ Preview

<div align="center">

<img src="https://placehold.co/900x450/071426/e6eef6?text=AutoFull+Web+Control" alt="AutoFull Web Control Preview" width="90%"/>

</div>

---

## 🧪 Estado actual

AutoFull V1.0 incluye:

* Control manual desde interfaz web.
* Comunicación ESP32 → Arduino UNO.
* Lectura de sensores.
* Detección de obstáculos.
* Evasión automática.
* OLED con ojos animados.
* UI responsive para navegador/celular.

---

## 🗺️ Roadmap

* [ ] Agregar modo automático completo.
* [ ] Mejorar el sistema de telemetría.
* [ ] Mostrar más datos en tiempo real desde la interfaz.
* [ ] Agregar control de velocidad desde la web.
* [ ] Mejorar feedback visual de obstáculos.
* [ ] Agregar capturas reales del proyecto.
* [ ] Documentar diagrama eléctrico.
* [ ] Agregar video demo.
* [ ] Separar configuración de pines en archivo dedicado.
* [ ] Mejorar protocolo Serial con mensajes más estructurados.

---

## 💡 Ideas futuras

* Control desde app móvil.
* Cámara ESP32-CAM.
* Modo patrulla autónomo.
* Control por joystick virtual.
* Indicador de batería.
* Mapeo básico de obstáculos.
* Personalidades OLED configurables.
* Dashboard más avanzado.

---

## ⚠️ Notas importantes

* Verificar correctamente las conexiones antes de alimentar motores.
* Usar una fuente externa adecuada para los motores.
* No alimentar motores directamente desde el Arduino.
* Compartir GND entre Arduino, ESP32 y driver de motores.
* Revisar el voltaje lógico entre Arduino UNO y ESP32 al comunicar por Serial.
* Ajustar los pines según el hardware utilizado.

---

## 👨‍💻 Autor

Desarrollado por **Vicente Aznar**.

<div align="center">

[![GitHub](https://img.shields.io/badge/GitHub-Aznar--7-181717?style=for-the-badge\&logo=github)](https://github.com/Aznar-7)
[![Portfolio](https://img.shields.io/badge/Portfolio-vicenteaznar.dev-10B981?style=for-the-badge\&logo=vercel\&logoColor=white)](https://vicenteaznar.dev)

</div>

---

## ⭐ Apoyo

Si te gustó el proyecto, podés dejar una estrella en el repositorio.

<div align="center">

### ⭐ AutoFull

**Robot Wi-Fi con sensores, control web y personalidad OLED.**

</div>
