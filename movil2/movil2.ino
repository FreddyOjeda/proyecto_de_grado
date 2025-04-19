#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <string>
#include "HT_DisplayUi.h"
#include "heltec.h"

// ─────────────────────────────────────────────
// OLED Display Configuration
// ─────────────────────────────────────────────
#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
#endif

DisplayUi ui(&display);

// ─────────────────────────────────────────────
// LoRa Configuration
// ─────────────────────────────────────────────
#define RF_FREQUENCY           915000000  // Hz
#define TX_OUTPUT_POWER        5          // dBm
#define LORA_BANDWIDTH         0          // 125 kHz
#define LORA_SPREADING_FACTOR  7
#define LORA_CODINGRATE        1          // 4/5
#define LORA_PREAMBLE_LENGTH   8
#define LORA_SYMBOL_TIMEOUT    0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON   false

#define RX_TIMEOUT_VALUE       1000       // ms
#define BUFFER_SIZE            32
#define MOVIL_ID               6          // Cambia este valor según el ID del transmisor

// ─────────────────────────────────────────────
// Transmisión escalonada (sin colisiones)
// ─────────────────────────────────────────────
const long wait_time    = ((MOVIL_ID * 1000) / 2) + 5000;
const long restart_time = 55000 - wait_time;

// ─────────────────────────────────────────────
// Variables globales
// ─────────────────────────────────────────────
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
std::string estado = "...";
bool canal_libre = false;
unsigned long tiempo_recibido = 0;

// ─────────────────────────────────────────────
// Pantalla: Frame 1 - ID del sensor
// ─────────────────────────────────────────────
void frame1(ScreenDisplay *display, DisplayUiState *state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  std::string mensaje = "Sensor #" + std::to_string(MOVIL_ID);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, mensaje.c_str());
}

// ─────────────────────────────────────────────
// Pantalla: Frame 2 - Estado actual
// ─────────────────────────────────────────────
void frame2(ScreenDisplay *display, DisplayUiState *state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, y, 128, estado.c_str());
}

FrameCallback frames[] = { frame1, frame2 };
int cantidad_frames = 2;

// ─────────────────────────────────────────────
// Funciones de evento LoRa
// ─────────────────────────────────────────────
void onPacketSent() {
  Serial.println("Paquete enviado correctamente");
  estado = "Envío exitoso";
}

void onTimeout() {
  Serial.println("Tiempo de espera para transmitir agotado");
}

void onReceive(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  if (strncmp((char *)payload, "START", size) == 0) {
    Serial.println("Señal de inicio recibida.");
    estado = "Señal recibida";
    tiempo_recibido = millis();
    canal_libre = true;
  } else {
    Serial.print("Paquete no reconocido: ");
    estado = "Cruce de información";
    for (uint16_t i = 1; i < size; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println("");
  }
}

// ─────────────────────────────────────────────
// Inicialización del sistema
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  // Inicializar eventos LoRa
  RadioEvents.TxDone = onPacketSent;
  RadioEvents.TxTimeout = onTimeout;
  RadioEvents.RxDone = onReceive;

  // Inicializar módulo LoRa
  Serial.println("Iniciando LoRa...");
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
  Serial.println("LoRa inicializado.");

  // Comenzar en modo escucha
  Radio.Rx(0);

  Serial.print("Esperando señal de inicio, tiempo de espera: ");
  Serial.print(wait_time);
  Serial.print(", Tiempo de reinicio: ");
  Serial.println(restart_time);

  estado = "Esperando...";

  // Configurar interfaz de pantalla
  ui.setTargetFPS(60);
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_UP);
  ui.setFrames(frames, cantidad_frames);
  ui.init();
}

// ─────────────────────────────────────────────
// Lógica del ciclo principal
// ─────────────────────────────────────────────
void loop() {
  unsigned long actual = millis();

  // Si ya pasó el tiempo de espera desde que se recibió START
  if (actual - tiempo_recibido >= wait_time && canal_libre) {
    canal_libre = false;
    estado = "Preparando envío";
    uint8_t payload[] = { MOVIL_ID, 'H', 'o', 'l', 'a', 'L', 'o', 'R', 'a' };
    Radio.Send(payload, sizeof(payload));
  }

  // Reiniciar recepción después del tiempo de reinicio
  if (actual - tiempo_recibido >= restart_time) {
    Radio.Rx(0);
  }

  Radio.IrqProcess();

  // Actualizar pantalla
  int tiempo_restante = ui.update();
  if (tiempo_restante > 0) {
    delay(tiempo_restante);
  }
}
