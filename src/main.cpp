#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS    D8
#define TFT_RST   D3
#define TFT_DC    D4

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(128, 160);

const unsigned char playerSprite [] PROGMEM = {
	0x30, 0x30, 0x78, 0x78, 0xfc, 0xfc, 0xcc
};

unsigned int playerX = 58;
unsigned int playerY = 145;

void setupScreen()
{
  tft.initR(INITR_18BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
}

void drawGameScreen()
{
  canvas.fillScreen(ST7735_BLACK);
  canvas.drawBitmap(playerX, playerY, playerSprite, 6, 7, ST7735_WHITE);
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 160);
}

void setup()
{  
  setupScreen();
}

void loop()
{
  drawGameScreen();
}
