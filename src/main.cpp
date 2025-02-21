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

const int DRONE_ROWS = 4;
const int DRONE_COLS = 8;

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

class GameEntity {
  protected:
    int posX;
    int posY;
    int width;
    int height;
    int speed;

  public:
    int getPosX() {
      return posX;
    }

    int getPosY() {
      return posY;
    }

    int getWidth() {
      return width;
    }

    int getHeight() {
      return height;
    }

    void setPosition(int x = 0, int y = 0) {
      posX = x;
      posY = y;
    }

    virtual void draw() = 0;
};

class Drone: public GameEntity {
  private:
    bool isActive;

  public:
    Drone() {
      width = 11;
      height = 11;
      speed = 5;
      isActive = true;
    }

    void disable() {
      isActive = false;
    }

    bool getActiveState() {
      return isActive;
    }

    void draw() {
      canvas.drawRGBBitmap(posX, posY, droneSprite, width, height);
    }
};

class Bullet: public GameEntity {
  private:
    bool isActive;

  public:
    Bullet() {
      width = 2;
      height = 6;
      speed = 5;
      isActive = false;
    }

    void setActive()
    {
      isActive = true;
    }
    
    bool getActiveState() {
      return isActive;
    }

    void move(int directionX, int directionY) {
      posX += directionX * speed;
      posY -= directionY * speed;
    }

    void disable() {
      isActive = false;
    }

    void draw() {
      if (getActiveState()) canvas.drawRect(posX, posY, width, height, ST7735_WHITE);
    }
};

class Player: public GameEntity {
  private:
    int score = 0;
    int lives = 0;
  
  public:
    Player() {
      width = 6;
      height = 7;
      speed = 3;
    }

    int getScore() {
      return score;
    }

    void setScore(int s) {
      score = s;
    }

    int getLives() {
      return lives;
    }

    void setLives(int l) {
      lives = l;
    }

    void move(int directionX) {
      if (directionX == -1 && posX < 3) return;
      if (directionX == 1 && posX > 120) return;

      posX += directionX * speed;
    }

    void shoot(Bullet& bullet) {
      if (! bullet.getActiveState()) {
        bullet.setActive();
        bullet.setPosition(posX + 2, posY - height);
      }
    }

    void draw() {
      canvas.drawBitmap(posX, posY, playerSprite, width, height, ST7735_WHITE);
    }
};

class Game {
  private:
    Player player;
    Drone drones[4][8];
    Bullet playerBullet;
    Bullet bullets[3];

  public:
    void init() {
      player.setPosition(58, 140);
      initDrones();
    }

    void initDrones() {
      for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 8; col++) {
          drones[row][col].setPosition(6 + col * (11 + 4), 30 + row * (11 + 4));
        }
      }
    }

    void handleInput()
    {
      if (digitalRead(BTN_LEFT) == LOW) {
        player.move(-1);
      }
    
      if (digitalRead(BTN_RIGHT) == LOW) {
        player.move(1);
      }

      if (digitalRead(BTN_SHOOT) == LOW) {
        player.shoot(playerBullet);
      }
    }

    void update()
    {
      if (playerBullet.getActiveState()) playerBullet.move(0, 1);
      if (playerBullet.getPosY() < 0) playerBullet.disable();
    }

    void checkCollisions()
    {
      for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 8; col++) {
          Drone& drone = drones[row][col];
          if (drone.getActiveState() &&
              playerBullet.getPosX() + playerBullet.getWidth() >= drone.getPosX() &&
              playerBullet.getPosX() <= drone.getPosX() + drone.getWidth() &&
              playerBullet.getPosY() + playerBullet.getHeight() >= drone.getPosY() + drone.getHeight() &&
              playerBullet.getPosY() <= drone.getPosY() + drone.getHeight()) {
            drone.disable();    
            playerBullet.disable();
            player.setScore(player.getScore() + 10);
          }
        }
      }
    }

    void drawDrones()
    {
      for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 8; col++) {
          if (drones[row][col].getActiveState()) {
            drones[row][col].draw();
          }
        }
      }
    }

    void render() {
      canvas.fillScreen(ST7735_BLACK);
      player.draw();
      playerBullet.draw();
      drawDrones();

      canvas.setCursor(5, 5);
      canvas.setTextColor(ST7735_WHITE);
      canvas.setTextSize(1);
      canvas.print(player.getScore());

      tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 128, 160);
    }
};

Game game;

void setupScreen()
{
  tft.initR(INITR_18BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
}

void setup()
{  
  Serial.begin(9600);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SHOOT, INPUT_PULLUP);
  setupScreen();
  game.init();
}

void loop()
{
  game.handleInput();
  game.update();
  game.checkCollisions();
  game.render();
}
