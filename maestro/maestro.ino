#define RE_PIN 2  // Pin de control de Recepción/Transmisión
#define DE_PIN 3  // Pin de control de Recepción/Transmisión

unsigned long previousMillis = 0;  // Variable para almacenar el último tiempo de envío
const unsigned long interval = 60000;  // Intervalo de 60 segundos (60000 milisegundos)

void setup() {
  pinMode(RE_PIN, OUTPUT);
  pinMode(DE_PIN, OUTPUT);
  
  digitalWrite(RE_PIN, LOW);  // Modo recepción al inicio
  digitalWrite(DE_PIN, LOW);

  Serial1.begin(9600, SERIAL_8N1, 16, 17);  // Inicializar el UART1 (puedes cambiar los pines RX=16 y TX=17 si es necesario)
  Serial.begin(115200);  // Comunicación con el monitor serie para depuración
}

void loop() {
  unsigned long currentMillis = millis();  // Obtener el tiempo actual

  // Verificar si han pasado 60 segundos desde el último envío
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Guardar el último tiempo en que se envió la señal

    // Enviar señal de sincronización a los moderadores
    digitalWrite(RE_PIN, HIGH);  // Cambiar a modo transmisión
    digitalWrite(DE_PIN, HIGH);

    Serial1.print("START");  // Enviar mensaje de sincronización a los moderadores
    Serial.println("Señal de sincronización enviada a moderadores.");
    
    delay(100);  // Pequeña pausa para asegurar que el mensaje se transmita (puedes ajustarlo si es necesario)

    digitalWrite(RE_PIN, LOW);  // Volver a modo recepción
    digitalWrite(DE_PIN, LOW);
  }
}
