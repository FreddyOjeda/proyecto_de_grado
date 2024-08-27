#include "heltec.h"
#include "HT_DisplayUi.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h" 
#include "images.h"

#ifdef WIRELESS_STICK_V3
static SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED); // addr , freq , i2c group , resolution , rst
#else
static SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
#endif

DisplayUi ui( &display );

void frame1(ScreenDisplay *display, DisplayUiState* state, int16_t x, int16_t y){
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "¡¡¡Hola...");
}
void frame2(ScreenDisplay *display, DisplayUiState* state, int16_t x, int16_t y){
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "¡¡¡Mundo...");
}
void frame3(ScreenDisplay *display, DisplayUiState* state, int16_t x, int16_t y){
  display->drawXbm(x, y, img_width, img_height, img_bits);
}

FrameCallback frames[] = {frame1, frame2, frame3};
int framesCant = 3;

void setup() {
  Serial.begin(115566);
  Serial.println();
  Serial.println();
  delay(100);

  ui.setTargetFPS(60);
  ui.setIndicatorPosition(BOTTOM);

  ui.setIndicatorDirection(LEFT_RIGHT);

  ui.setFrameAnimation(SLIDE_UP);

  ui.setFrames(frames, framesCant);
  ui.init();
}

void loop() {
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
}
