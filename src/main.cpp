#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS    D8
#define TFT_RST   D3
#define TFT_DC    D4

#define BTN_LEFT  D1
#define BTN_RIGHT D2
#define BTN_SHOOT 3

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(128, 160);

const unsigned char playerSprite [] PROGMEM = {
	0x30, 0x30, 0x78, 0x78, 0xfc, 0xfc, 0xcc
};

const uint16_t droneSprite [] PROGMEM = {
	0x001f, 0x001f, 0x001f, 0x001f, 0x0000, 0x0000, 0x0000, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x0000, 
	0x0000, 0x0000, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x0000, 0x0000, 0x0000, 0x001f, 0x001f, 0x001f, 
	0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x0000, 0x0000, 0x0000, 0x001f, 
	0xffff, 0xffff, 0xffff, 0x001f, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x001f, 0xffff, 0xffff, 0xffff, 0x001f, 0x0000, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x001f, 0xffff, 0xffff, 0xffff, 0x001f, 0x0000, 0x0000, 0x0000, 0x001f, 0x001f, 0x001f, 
	0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x0000, 0x0000, 0x0000, 0x001f, 
	0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x0000, 0x0000, 0x0000, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 0x001f, 
	0x001f, 0x001f, 0x0000, 0x0000, 0x0000, 0x001f, 0x001f, 0x001f, 0x001f
};

unsigned int playerX = 58;
unsigned int playerY = 145;

unsigned int bulletX = 0;
unsigned int bulletY = 0;
bool bulletActive = false;

struct Drone {
  unsigned int posX;
  unsigned int posY;
  bool isActive;
};

struct Drone droneList[32];

void setupScreen()
{
  tft.initR(INITR_18BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
}

void resetBullet()
{
  bulletX = 0;
  bulletY = 0;
  bulletActive = false;
}

void initDrones()
{
  int droneIndex = 0;

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 8; col++) {
      droneList[droneIndex].posX = 6 + col * 15;
      droneList[droneIndex].posY = 20 + row * 15;

      droneList[droneIndex].isActive = true;

      droneIndex++;
    }
  }
}

void moveBullet()
{
  Serial.println(bulletY);

  if (bulletActive && bulletY > 3) {
    bulletY -= 3;
    return;
  }

  resetBullet();
}

void checkCollisions()
{
  int droneListLength = sizeof(droneList) / sizeof(droneList[0]);

  for (int i = 0; i < droneListLength; i++) {
    if (droneList[i].isActive &&
      bulletX + 2 >= droneList[i].posX && 
      bulletX <= droneList[i].posX + 11 && 
      bulletY + 4 >= droneList[i].posY && 
      bulletY <= droneList[i].posY + 11) {
      droneList[i].isActive = false;
      resetBullet();
    }
  }
}

void drawGameScreen()
{
  int droneListLength = sizeof(droneList) / sizeof(droneList[0]);

  canvas.fillScreen(ST7735_BLACK);
  canvas.drawBitmap(playerX, playerY, playerSprite, 6, 7, ST7735_WHITE);

  if (bulletActive) {
    canvas.fillRect(bulletX, bulletY, 2, 4, ST7735_WHITE);
  }

  for (int i = 0; i < droneListLength; i++) {
    if (droneList[i].isActive) {
    canvas.drawRGBBitmap(droneList[i].posX, droneList[i].posY, droneSprite, 11, 11);
    }
  }

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 160);
}

void handleButtonInputs()
{
  if (digitalRead(BTN_LEFT) == LOW && playerX > 3) {
    playerX -= 3;
  }

  if (digitalRead(BTN_RIGHT) == LOW && playerX < 120) {
    playerX += 3;
  }

  if (digitalRead(BTN_SHOOT) == LOW && bulletActive == false) {
    bulletX = playerX + 2;
    bulletY = playerY - 8;
    bulletActive = true;
  }
}

void setup()
{  
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SHOOT, INPUT_PULLUP);

  initDrones();
  setupScreen();
}

void loop()
{
  handleButtonInputs();
  moveBullet();
  checkCollisions();
  drawGameScreen();
}
