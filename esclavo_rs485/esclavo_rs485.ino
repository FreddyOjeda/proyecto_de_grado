#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_SSD1306Wire.h"
#include <string>
#include "HT_DisplayUi.h"
#include "heltec.h"

// Configuración para la pantalla OLED
#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);  // addr, freq, i2c group, resolution, rst
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr, freq, i2c group, resolution, rst
#endif

DisplayUi ui(&display);

// Configuración LoRa
#define RF_FREQUENCY 915000000  // Hz
#define TX_OUTPUT_POWER 4       // dBm
#define LORA_BANDWIDTH 0        // [0: 125 kHz]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1       // [1: 4/5]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 32  // Tamaño del payload

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

std::string ids = "...";
std::string buffer = ""; // Buffer para almacenar los datos recibidos de los móviles
int paquetes = 0;

static RadioEvents_t RadioEvents;
int16_t rssi, rxSize;
bool chanel_free = true;

void frame1(ScreenDisplay *display, DisplayUiState *state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "Sistema Iniciado");
  display->drawStringMaxWidth(0 + x, y, 128, "Moderador #2");
}

void frame2(ScreenDisplay *display, DisplayUiState *state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  std::string mensaje = "Paquetes Recibidos: " + std::to_string(paquetes);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, mensaje.c_str());
  std::string mensaje2 = "IDs de Sensores: " + ids;
  display->drawStringMaxWidth(0 + x, y, 128, mensaje2.c_str());
}

FrameCallback frames[] = { frame1, frame2 };
int framesCant = 2;

// Configuración para la comunicación RS-485 usando Serial1 en ESP32
#define RS485_TX_PIN 17
#define RS485_RX_PIN 16
#define RS485_ENABLE_PIN 4

void configureRS485() {
  Serial1.begin(9600, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);  // Configuración del puerto serial para RS-485
  pinMode(RS485_ENABLE_PIN, OUTPUT);
  digitalWrite(RS485_ENABLE_PIN, LOW);  // Pone RS-485 en modo recepción
}

void sendRS485(const String &message) {
  digitalWrite(RS485_ENABLE_PIN, HIGH); // Habilita transmisión
  Serial1.print(message);               // Envía el mensaje
  Serial1.flush();
  digitalWrite(RS485_ENABLE_PIN, LOW);  // Vuelve a modo recepción
}

String receiveRS485() {
  if (Serial1.available()) {
    return Serial1.readString();
  }
  return "";
}

// Funciones LoRa
void onReceive(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  paquetes++;
  buffer += std::to_string(payload[0]) + ":" + std::string((char *)payload + 1, size - 1) + "\n"; // Almacena el ID y los datos recibidos
  
  Serial.print("Paquete recibido de ID: ");
  Serial.print(payload[0]);
  Serial.print(" Contenido: ");
  for (uint16_t i = 1; i < size; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print(" con RSSI ");
  Serial.print(rssi);
  Serial.print(" y SNR ");
  Serial.println(snr);
}

void sendPacket() {
  Serial.println("Envio Correcto");
  buffer = "";  // Reinicia el buffer
  paquetes = 0;
  ids = "";
  chanel_free = true;
  Radio.Rx(0);  // Prepara para recibir cualquier paquete de vuelta
}

void setupLoRa() {
  RadioEvents.RxDone = onReceive;
  RadioEvents.TxDone = sendPacket;

  Serial.println("Iniciando LoRa...");
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR, LORA_CODINGRATE, 0,
                    LORA_PREAMBLE_LENGTH, LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
  Serial.println("LoRa inicializado.");
  Radio.Rx(0);  // Prepara para recibir
}

void setupDisplay() {
  ui.setTargetFPS(60);
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_UP);
  ui.setFrames(frames, framesCant);
  ui.init();
}

static unsigned long lastSignalTime = 0;

void setup() {
  Serial.begin(115200);  // Comunicación por USB para depuración
  configureRS485();       // Configurar RS-485
  setupLoRa();            // Configurar LoRa
  setupDisplay();         // Configurar pantalla OLED
}

void loop() {
  unsigned long currentTime = millis();

  // Actualiza la pantalla OLED
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);  // No bloquea si hay tiempo disponible
  }

  // Recibe mensaje del maestro por RS-485
  String command = receiveRS485();
  if (command == "START") {
    Serial.println("Señal START recibida del maestro.");

    // Enviar la señal START por LoRa a los móviles
    uint8_t startSignal[] = { 'S', 'T', 'A', 'R', 'T' };
    Radio.Send(startSignal, sizeof(startSignal));
    Serial.println("Señal START enviada a móviles.");
    
    // Esperar recepción de datos de móviles durante 60 segundos
    lastSignalTime = millis();
  }

  // Después de 60 segundos, enviar datos al maestro
  if (currentTime - lastSignalTime >= 60000 && !chanel_free) {
    sendRS485(String("DATA:") + buffer.c_str());  // Enviar buffer al maestro por RS-485
    Serial.println("Datos enviados al maestro:");
    Serial.println(String("DATA:") + buffer.c_str());
    buffer = "";  // Limpiar buffer después de enviar
    lastSignalTime = currentTime;
    chanel_free = true;
  }

  // Procesa interrupciones de LoRa
  Radio.IrqProcess();
}
