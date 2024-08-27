#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"

#define RF_FREQUENCY 915000000  // Hz
#define TX_OUTPUT_POWER 4       // dBm

#define LORA_BANDWIDTH 0         // [0: 125 kHz,
#define LORA_SPREADING_FACTOR 7  // [SF7..SF12]
#define LORA_CODINGRATE 1        // [1: 4/5,
#define LORA_PREAMBLE_LENGTH 8   // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0    // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 32  // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

SSD1306Wire oledDisplay(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);  // addr , freq , i2c group , resolution , rst

static RadioEvents_t RadioEvents;
int16_t rssi, rxSize;
bool chanel_free = true;

void onReceive(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  rssi = rssi;
  rxSize = size;
  memcpy(rxpacket, payload, size);
  rxpacket[size] = '\0';
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
  chanel_free = true;
}

void sendPacket() {
  Serial.println("Envio Correcto");
  chanel_free = true;
  Radio.Rx(0);  // Preparar para recibir cualquier paquete de vuelta
}

void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  rssi = 0;

  RadioEvents.RxDone = onReceive;
  RadioEvents.TxDone = sendPacket;

  Serial.println("Iniciando LoRa...");
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Serial.println("LoRa inicializado.");

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  oledDisplay.init();
  oledDisplay.setFont(ArialMT_Plain_10);
}

void loop() {
  static unsigned long lastSignalTime = 0;
  unsigned long currentTime = millis();

  oledDisplay.clear();
  oledDisplay.drawString(0, 0, "LoRa Iniciado");
  oledDisplay.drawString(0, 0, "Moderador #1");

  // Envía la señal inicial cada minuto
  if (currentTime - lastSignalTime >= 60000) {
    uint8_t startSignal[] = { 'S', 'T', 'A', 'R', 'T' };
    Radio.Send(startSignal, sizeof(startSignal));
    Serial.println("Señal de inicio enviada.");
    chanel_free = false;
    lastSignalTime = currentTime;
  }

  if (chanel_free) {
    chanel_free = false;
    Radio.Rx(0);  // Preparar para recibir cualquier paquete de vuelta
    Serial.println("Listo para recibir");
  }

  Radio.IrqProcess();
}
