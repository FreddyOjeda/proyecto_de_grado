#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <string>

#include "HT_DisplayUi.h"
#include "heltec.h"

#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);  // addr , freq , i2c group , resolution , rst
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);  // addr , freq , i2c group , resolution , rst
#endif

DisplayUi ui(&display);

#define RF_FREQUENCY 915000000  // Hz
#define TX_OUTPUT_POWER 5       // dBm

#define LORA_BANDWIDTH 0         // [0: 125 kHz,
#define LORA_SPREADING_FACTOR 7  // [SF7..SF12]
#define LORA_CODINGRATE 1        // [1: 4/5,
#define LORA_PREAMBLE_LENGTH 8   // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0    // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 32
#define MOVIL_ID 6  // Cambia este valor según el ID del transmisor

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

const long wait_time = ((MOVIL_ID * 1000) / 2) + 5000;  //este sera el tiempo de espera para iniciar el envio
const long restart_time = 55000 - wait_time;            //este sera el tiempo que debe esperar el nodo para volver a escuchar

static RadioEvents_t RadioEvents;

std::string estado = "...";

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
bool chanel_free = false;

void sendPacket() {
  Serial.println("Paquete enviado correctamente");
  estado = "Envio Exitoso";
}

unsigned long rx_millis = 0;
void onReceive(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  if (strncmp((char *)payload, "START", size) == 0) {
    Serial.println("Señal de inicio recibida.");
    estado = "Señal Recibida";

    rx_millis = millis();
    chanel_free = true;

  } else {
    Serial.print("Paquete no reconocido: ");
    estado = "cruce de informacion";
    for (uint16_t i = 1; i < size; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println("");
  }
}

void timeOut() {
  Serial.println("Tiempo de espera para transmitir agotado");
}

void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = sendPacket;
  RadioEvents.TxTimeout = timeOut;
  RadioEvents.RxDone = onReceive;

  Serial.println("Iniciando LoRa...");
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
  Serial.println("LoRa inicializado.");

  Radio.Rx(0);  // Preparar para recibir la señal de inicio
  Serial.print("Esperando señal de inicio, tiempo de espera: ");
  Serial.print(wait_time);
  Serial.print(", Tiempo de reinicio: ");
  Serial.println(restart_time);

  estado = "Esperando...";

  ui.setTargetFPS(60);
  ui.setIndicatorPosition(BOTTOM);

  ui.setIndicatorDirection(LEFT_RIGHT);

  ui.setFrameAnimation(SLIDE_UP);

  ui.setFrames(frames, framesCant);
  ui.init();
}


void loop() {
  unsigned long current_millis = millis();
  if (current_millis - rx_millis >= wait_time && chanel_free == true) {

    chanel_free = false;

    estado = "Preparando envio";
    uint8_t payload[] = { MOVIL_ID, 'H', 'o', 'l', 'a', 'L', 'o', 'R', 'a' };  // Ejemplo con ID 1
    Radio.Send(payload, sizeof(payload));
  }
  if (current_millis - rx_millis >= restart_time) {
    Radio.Rx(0);
  }
  Radio.IrqProcess();


  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
}
