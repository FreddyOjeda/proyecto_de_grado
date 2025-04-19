# 🔗 Red de Sensores Inalámbricos (WSN) con LoRa y ESP32 Heltec WiFi LoRa V3

Este proyecto implementa una **Red de Sensores Inalámbricos (WSN)** utilizando módulos **ESP32 Heltec WiFi LoRa V3**. Los dispositivos se comunican mediante **LoRa** en topología tipo maestro-esclavo, integrando además un sistema de monitoreo por pantalla OLED. El diseño permite futura expansión hacia comunicación **RS485 cableada** sin afectar el sistema inalámbrico actual.

---

## 📁 Estructura del Proyecto

```plaintext
├───Control_pantalla         # Código de prueba de pantalla OLED
├───esclavo                  # Nodo receptor en la red LoRa
├───esclavo_rs485            # Versión experimental con soporte RS485
├───maestro                  # Nodo maestro para conexión RS485
├───movil                    # Nodo transmisor LoRa
└───movil2                   # Segundo nodo transmisor LoRa
```

---

## 🔧 Requisitos

- Módulo **Heltec WiFi LoRa V3 (ESP32)**
- Arduino IDE o PlatformIO
- Librerías:
  - `LoRaWan_APP.h` (de Heltec)
  - `heltec.h`
  - `SSD1306Wire` (OLED)
  - `HT_DisplayUi` (para control de interfaz de pantalla)
- Cable microUSB tipo C
- Antenas LoRa conectadas

---

## ⚙️ Configuración Inicial

1. **Instalar el paquete de tarjetas ESP32** en el **Arduino IDE**:
   - En el Gestor de tarjetas, instala: `esp32 by Espressif Systems`.

2. **Seleccionar la placa adecuada**:
   - `Tools > Board > Heltec WiFi LoRa 32 (V3)`

3. **Instalar librerías necesarias**:
   - Desde el Gestor de librerías o manualmente:
     - `Heltec ESP32`
     - `OLED SSD1306`
     - `HT_DisplayUi` (si no está disponible, usar versión incluida en Heltec examples)

4. **Conectar la antena LoRa antes de encender** la placa para evitar daño al transceptor.

---

## 📦 Descripción de los módulos

### `Control_pantalla`

> Código simple para validar el funcionamiento de la pantalla OLED incluida en los módulos Heltec. Muestra texto básico en pantalla para comprobar contraste, conexión y respuesta.

---

### `movil` y `movil2`

> Nodos transmisores de la red LoRa. Envían paquetes de datos de manera escalonada para evitar colisiones. Utilizan una estrategia de retardo basado en su identificador (`MOVIL_ID`).

#### Características:

- Envío de datos tras recibir señal de sincronización `"START"` desde el esclavo.
- Uso de pantalla OLED para mostrar:
  - ID del sensor
  - Estado del envío (esperando, recibido, enviado, conflicto)
- Algoritmo anti-colisión usando retardo escalonado.

#### Parámetros configurables:

- `MOVIL_ID`: Define el ID del transmisor (distinto en `movil` y `movil2`)
- `wait_time`: Calculado en base al `MOVIL_ID`
- `restart_time`: Tiempo total de la ventana de transmisión

---

### `esclavo`

> Nodo receptor que actúa como coordinador de la red LoRa. Envía periódicamente una señal de sincronización (`"START"`) y recibe datos de los nodos móviles.

#### Características:

- Transmite señal de inicio cada 60 segundos.
- Escucha durante una ventana de tiempo predefinida.
- Muestra los paquetes recibidos en consola serial.
- Puede visualizar información en pantalla OLED sobre los paquetes recibidos.

---

### `esclavo_rs485` *(en desarrollo)*

> Versión experimental del nodo esclavo, con capacidad para integrarse en redes cableadas utilizando el protocolo RS485. Mantiene la lógica de sincronización y recepción, pero con miras a adaptarse a un entorno híbrido (LoRa + RS485).

---

### `maestro` *(en desarrollo)*

> Nodo maestro para establecer comunicación con `esclavo_rs485` en sistemas cableados tipo maestro-esclavo. Se está diseñando como intermediario entre sistemas embebidos y SCADA/PC vía RS485.

---

## 🔄 Topología del sistema

```plaintext
+-----------------+          LoRa          +-----------------+          LoRa           +-----------------+
|     movil       | <------------------->  |     esclavo     |  <------------------->  |     movil2      |
|  (Sensor #1)    |                       | (Coordinador)    |                        |  (Sensor #2)    |
+-----------------+                       +------------------+                        +-----------------+

    \                                                                  /
     \--- en desarrollo: conexión RS485 (maestro <--> esclavo_rs485) /
```

---

## 📷 Ejemplo de Salida en Consola Serial

```plaintext
Esperando señal de inicio, tiempo de espera: 8000, Tiempo de reinicio: 47000
Señal de inicio recibida.
Preparando envío
Paquete enviado correctamente
```

---

## 📌 Notas adicionales

- Todos los códigos están optimizados para su uso en placas **Heltec WiFi LoRa V3**.
- La red funciona a una frecuencia de **915 MHz**.  asegúrate de ajustar `RF_FREQUENCY` a **868 MHz** si es necesario.
- El sistema de retardo escalonado evita interferencias entre múltiples nodos transmisores, sin embargo se puede modificar dependiendo la cantidad de dispositivos moviles simultaneos.
- Se recomienda probar cada módulo por separado antes de implementarlos juntos.

---

## 🧑‍💻 Autor

**Freddy Ojeda**  
Ingeniero de Sistemas y Computación  
Universidad Pedagógica y Tecnológica de Colombia  
📍 Sogamoso, Boyacá  
✉️ freddy.ojeda@uptc.edu.co

---

## 📜 Licencia

Este proyecto es de libre uso para fines educativos y de investigación. Se agradece atribución al autor en proyectos derivados.
