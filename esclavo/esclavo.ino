#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include <string>
#include "HT_DisplayUi.h"
#include "heltec.h"

// Configuración de pantalla OLED según el modelo de placa
#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
#endif

DisplayUi ui(&display);

// Parámetros de configuración LoRa
#define RF_FREQUENCY 915000000     // Frecuencia de operación (Hz)
#define TX_OUTPUT_POWER 4          // Potencia de transmisión (dBm)
#define LORA_BANDWIDTH 0           // 0: 125 kHz
#define LORA_SPREADING_FACTOR 7    // Entre SF7 y SF12
#define LORA_CODINGRATE 1          // 1: 4/5
#define LORA_PREAMBLE_LENGTH 8     // Longitud del preámbulo
#define LORA_SYMBOL_TIMEOUT 0      // Timeout de símbolo
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000      // Tiempo de espera de recepción
#define BUFFER_SIZE 32             // Tamaño del buffer de datos

// Buffers de transmisión y recepción
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

// Variables para mostrar en la pantalla
std::string ids = "...";
int paquetes = 0;

// Frame 1: mensaje inicial
void frame1(ScreenDisplay *display, DisplayUiState *state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, y, 128, "Moderador #2");
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "Sistema Iniciado");
}

// Frame 2: datos dinámicos de paquetes e IDs
void frame2(ScreenDisplay *display, DisplayUiState *state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);

  std::string mensaje = "Paquetes Recibidos: " + std::to_string(paquetes);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, mensaje.c_str());

  std::string mensaje2 = "IDs de Sensores: " + ids;
  display->drawStringMaxWidth(0 + x, y, 128, mensaje2.c_str());
}

// Registro de frames para la interfaz de pantalla
FrameCallback frames[] = { frame1, frame2 };
const int framesCant = sizeof(frames) / sizeof(frames[0]);

// Variables relacionadas con LoRa
static RadioEvents_t RadioEvents;
int16_t rssi;
int rxSize;
bool chanel_free = true;  // Control de recepción

// Manejador de recepción de paquetes
void onReceive(uint8_t *payload, uint16_t size, int16_t rssiValue, int8_t snr) {
  if (size == 0) return;  // Validación de tamaño

  rssi = rssiValue;
  rxSize = size;

  // Copia el contenido recibido al buffer y lo termina en null
  memcpy(rxpacket, payload, size);
  rxpacket[size] = '\0';

  paquetes++;

  // Agrega el ID del sensor al string (asume que payload[0] es ID)
  ids += " " + std::to_string(payload[0]);

  // Controla que el string de IDs no se vuelva demasiado largo
  if (ids.length() > 50) {
    ids = "...";
  }

  // Imprime en consola el contenido del paquete
  Serial.print("Paquete recibido de ID: ");
  Serial.print(payload[0]);
  Serial.print(" | Contenido: ");
  for (uint16_t i = 1; i < size; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print(" | RSSI: ");
  Serial.print(rssi);
  Serial.print(" | SNR: ");
  Serial.println(snr);
}

// Manejador cuando la transmisión termina
void sendPacket() {
  Serial.println("Señal de inicio enviada correctamente.");

  // Reinicia contadores
  paquetes = 0;
  ids = "...";
  chanel_free = true;

  // Prepara para recibir respuestas
  Radio.Rx(0);
}

// Configuración inicial
void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  rssi = 0;

  // Configura los eventos de LoRa
  RadioEvents.RxDone = onReceive;
  RadioEvents.TxDone = sendPacket;

  Serial.println("Iniciando LoRa...");
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Serial.println("LoRa inicializado.");

  // Configura recepción LoRa
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  // Configuración de interfaz de pantalla
  ui.setTargetFPS(60);
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_UP);
  ui.setFrames(frames, framesCant);
  ui.init();
}

// Control de tiempo para enviar la señal START cada minuto
static unsigned long lastSignalTime = 0;

// Bucle principal
void loop() {
  unsigned long currentTime = millis();

  // Actualiza la interfaz gráfica
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);  // Ahorro de energía si hay tiempo libre
  }

  // Cada minuto se envía una señal de inicio
  if (currentTime - lastSignalTime >= 60000) {
    uint8_t startSignal[] = { 'S', 'T', 'A', 'R', 'T' };
    Radio.Send(startSignal, sizeof(startSignal));

    chanel_free = false;  // No escuchar mientras se espera respuesta
    lastSignalTime = currentTime;
  }

  // Si el canal está libre, se pone en modo recepción
  if (chanel_free) {
    chanel_free = false;
    Radio.Rx(0);
    Serial.println("Listo para recibir respuestas.");
  }

  // Procesa interrupciones del módulo LoRa
  Radio.IrqProcess();
}
