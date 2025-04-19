#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <string>

#include "HT_DisplayUi.h"
#include "heltec.h"

// Configuración de la pantalla OLED según la placa utilizada
#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
#endif

DisplayUi ui(&display);

// ========================== Configuración LoRa ==========================
#define RF_FREQUENCY 915000000      // Frecuencia de operación (Hz)
#define TX_OUTPUT_POWER 5           // Potencia de transmisión (dBm)
#define LORA_BANDWIDTH 0            // 0: 125 kHz
#define LORA_SPREADING_FACTOR 7     // SF7
#define LORA_CODINGRATE 1           // 4/5
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 1000       // Tiempo de espera en recepción (ms)
#define BUFFER_SIZE 32
#define MOVIL_ID 12                 // Cambia este valor según el ID del transmisor

// ========================== Variables globales ==========================
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
std::string estado = "...";

const long wait_time = ((MOVIL_ID * 1000) / 2) + 5000;  // Tiempo de espera para enviar después de recibir la señal
const long restart_time = 55000 - wait_time;            // Tiempo antes de volver a escuchar

static RadioEvents_t RadioEvents;

bool chanel_free = false;  // Bandera para indicar si se puede transmitir
unsigned long rx_millis = 0;  // Marca de tiempo de la última recepción

// ========================== Frames para OLED ==========================
void frame1(ScreenDisplay *display, DisplayUiState *state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  std::string mensaje = "Sensor #" + std::to_string(MOVIL_ID);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, mensaje.c_str());
}

void frame2(ScreenDisplay *display, DisplayUiState *state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, y, 128, estado.c_str());
}

FrameCallback frames[] = { frame1, frame2 };
int framesCant = 2;

// ========================== Callbacks de LoRa ==========================
void sendPacket() {
  Serial.println("Paquete enviado correctamente");
  estado = "Envio Exitoso";
}

void onReceive(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  if (strncmp((char *)payload, "START", size) == 0) {
    Serial.println("Señal de inicio recibida.");

    if (rssi >= -100) {
      Serial.println("Señal óptima");
      estado = "Señal Recibida";
      rx_millis = millis();
      chanel_free = true;
    } else {
      Serial.println("Señal NO óptima");
      estado = "Señal baja para transmitir";
    }

  } else {
    Serial.print("Paquete no reconocido: ");
    estado = "Cruce de información";
    for (uint16_t i = 1; i < size; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }
}

void timeOut() {
  Serial.println("Tiempo de espera para transmitir agotado");
}

// ========================== Setup ==========================
void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  // Configurar callbacks de eventos LoRa
  RadioEvents.TxDone = sendPacket;
  RadioEvents.TxTimeout = timeOut;
  RadioEvents.RxDone = onReceive;

  // Inicializar radio LoRa
  Serial.println("Iniciando LoRa...");
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
  Serial.println("LoRa inicializado.");

  // Iniciar en modo recepción
  Radio.Rx(0);

  Serial.print("Esperando señal de inicio, tiempo de espera: ");
  Serial.print(wait_time);
  Serial.print(", Tiempo de reinicio: ");
  Serial.println(restart_time);
  estado = "Esperando...";

  // Configurar UI de pantalla
  ui.setTargetFPS(60);
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_UP);
  ui.setFrames(frames, framesCant);
  ui.init();
}

// ========================== Loop principal ==========================
void loop() {
  unsigned long current_millis = millis();

  // Si se recibió la señal y se esperó el tiempo necesario, enviar
  if (current_millis - rx_millis >= wait_time && chanel_free) {
    chanel_free = false;
    estado = "Preparando envio";

    uint8_t payload[] = { MOVIL_ID, 'H', 'o', 'l', 'a', 'L', 'o', 'R', 'a' };
    Radio.Send(payload, sizeof(payload));
  }

  // Reanudar recepción después del tiempo definido
  if (current_millis - rx_millis >= restart_time) {
    Radio.Rx(0);
  }

  // Procesar interrupciones LoRa
  Radio.IrqProcess();

  // Actualizar pantalla
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);
  }
}
