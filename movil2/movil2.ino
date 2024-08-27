#include "LoRaWan_APP.h"
#include "Arduino.h"
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

double txNumber;
bool chanel_free = false;
unsigned long delayTime;

static RadioEvents_t RadioEvents;

void sendPacket() {
  Serial.println("Paquete enviado correctamente");
  chanel_free = false;  // Esperar la próxima señal de inicio
  Radio.Sleep();        // Dormir la radio después de la transmisión
}

void onReceive(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  if (strncmp((char *)payload, "START", size) == 0) {
    Serial.println("Señal de inicio recibida.");
    delayTime = (10000 / MOVIL_ID) + 10000;  // Calcula el tiempo de retardo
    chanel_free = true;
    Radio.Sleep();  // Dormir la radio durante 50 segundos
    delay(50000);   // Esperar 50 segundos
    Radio.Standby(); // Después de 50 segundos, vuelve a poner la radio en modo de espera
  } else {
    Serial.print("Paquete no reconocido: ");
    for (uint16_t i = 1; i < size; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println("_______________________________________");
    Radio.Rx(0);  // Preparar para recibir la señal de inicio
  }
}

void timeOut() {
  Radio.Sleep();
  Serial.println("Tiempo de espera para transmitir agotado");
}

void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  txNumber = 0;

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
  Serial.println("Esperando señal de inicio...");
}

void loop() {
  if (chanel_free) {
    delay(delayTime);  // Espera el tiempo específico basado en el ID
    Serial.print("Enviando --- ");

    uint8_t payload[] = { MOVIL_ID, 'H', 'o', 'l', 'a', ' ', 'L', 'o', 'R', 'a' };  // Ejemplo con ID 1

    Radio.Send(payload, sizeof(payload));
  }
  Radio.IrqProcess();
}
