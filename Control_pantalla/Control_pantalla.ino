#include "heltec.h"
#include "HT_DisplayUi.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h" 
#include "images.h"

// Configuración del display según el modelo
#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
#endif

// Inicializa la interfaz de usuario
DisplayUi ui(&display);

// === Funciones de cada "frame" para mostrar en pantalla ===

// Frame 1: mensaje de bienvenida
void frame1(ScreenDisplay* display, DisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(x, y + 10, 128, "¡¡¡Hola...");
}

// Frame 2: segundo mensaje
void frame2(ScreenDisplay* display, DisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(x, y + 10, 128, "¡¡¡Mundo...");
}

// Frame 3: imagen en pantalla
void frame3(ScreenDisplay* display, DisplayUiState* state, int16_t x, int16_t y) {
  display->drawXbm(x, y, img_width, img_height, img_bits);
}

// Arreglo de frames y cantidad
FrameCallback frames[] = { frame1, frame2, frame3 };
const int frameCount = sizeof(frames) / sizeof(frames[0]);

// === Setup principal ===
void setup() {
  Serial.begin(115200);  // Velocidad de baud estándar y suficiente
  delay(100);

  // Configuración de la UI
  ui.setTargetFPS(60);                     // Fotogramas por segundo
  ui.setIndicatorPosition(BOTTOM);        // Indicador en la parte inferior
  ui.setIndicatorDirection(LEFT_RIGHT);   // Dirección de desplazamiento
  ui.setFrameAnimation(SLIDE_UP);         // Tipo de animación entre frames
  ui.setFrames(frames, frameCount);       // Asigna los frames definidos
  ui.init();                              // Inicializa la UI
}

// === Loop principal ===
void loop() {
  int remainingTimeBudget = ui.update(); // Actualiza la interfaz

  // Si hay tiempo restante, se puede usar para otras tareas
  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);
  }
}
