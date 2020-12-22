/*
  A simple two-player Pong game.
*/
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

#define RESET_BUTTON WIO_KEY_A // top right

//Player 2 controls
// Buttons connected to GROVE plugs
//#define SUP_BUTTON BCM3
//#define SDOWN_BUTTON BCM27
// Joystick
#define SUP_BUTTON WIO_5S_UP
#define SDOWN_BUTTON WIO_5S_DOWN

// Player 1 controls
#define UP_BUTTON WIO_KEY_B // left
#define DOWN_BUTTON WIO_KEY_C // middle

#define PLAYER1 0
#define PLAYER2 1

#define TFT_WIDTH 320
#define TFT_HEIGHT 240

#define YOFFSET 50 // court offset in pixels from 0th row.
#define FRAMEWIDTH 5 // pixel width of court frame

class Ball;
class Court;

class Paddle {
  public:
    Paddle(int width, int height, int color, int x) : color(color), w(width), h(height), x(x) {
      this->y = (TFT_HEIGHT - height) / 2; //  center paddle vertically on screen.
    }
    void moveUp(int pixels);
    void moveDown(int pixels);
    void draw();
  private:
    int x, y, w, h, color;
    friend class Ball;
};

void Paddle::moveUp(int pixels) {
  this->y -= pixels;
  if (this->y < YOFFSET + FRAMEWIDTH) {
    this->y = YOFFSET + FRAMEWIDTH;
  }
}
void Paddle::moveDown(int pixels) {
  this->y += pixels;
  if (this->y > TFT_HEIGHT - this->h) { // paddle off the screen?
    this->y -= pixels; // reverse the op.
  }
}
void Paddle::draw() {
  spr.fillRect(this->x, this->y, this->w, this->h, this->color);
}

class Ball {
  public:
    Ball(int d, int color): color(color), d(d) {
      // Ball is square d is both w/h
      reset();
    }
    void update();
    void reset();
    void bounce();
    void draw();
    boolean collide(Paddle &p);
    int x;
  private:
    int y, d, color;
    int vx, vy; // velocity
    friend class Court;
};
boolean Ball::collide(Paddle &p) {
  return this->x < p.x + p.w &&
         this->x + this->d > p.x &&
         this->y < p.y + p.h &&
         this->y + this->d > p.y;
}

void Ball::reset() {
  // center ball on screen
  this->x = TFT_WIDTH / 2;
  this->y = TFT_HEIGHT / 2;
  // with a random velocity
  this->vx = random(4, 8);
  if (random(1)) this->vx = -(this->vx);
  this->vy = random(-8, 8);
}
void Ball::bounce() {
  // Hit a paddle reverse direction with random bounce.
  this->vx = -(this->vx);
  this->vy = random(-8, 8);
}
void Ball::update() {
  this->x += this->vx;
  this->y += this->vy;
}
void Ball::draw() {
  spr.fillRect(this->x, this->y, this->d, this->d, this->color);
}

class Court {
  public:
    Court() {}
    void draw();
    boolean collide(Ball &ball);
    void incScore(uint8_t i) {
      this->score[i]++;
    }
  private:
    int score[2];
};

boolean Court::collide(Ball &ball) {
  if (ball.x > TFT_WIDTH || ball.x < 0) {
    this->incScore(ball.x > TFT_WIDTH ? PLAYER2 : PLAYER1);
    return true; //end game
  } else if (ball.y > TFT_HEIGHT - FRAMEWIDTH || ball.y < YOFFSET + FRAMEWIDTH) { // Hit a wall?
    ball.vy = -ball.vy;
  }
  return false;
}
void Court::draw() {
  spr.setFreeFont(&FreeSansBold12pt7b);
  spr.setTextColor(TFT_CYAN);
  spr.drawString("Player 1:", 0, 5, 1);
  spr.drawString("Player 2:", 170, 5, 1);
  spr.setTextColor(TFT_WHITE);
  spr.drawNumber(score[PLAYER1], 120, 5);
  spr.drawNumber(score[PLAYER2], 285, 5);

  for (int i = 0; i < FRAMEWIDTH; i++)
    spr.drawRect(i, i + YOFFSET, TFT_WIDTH - i * 2, TFT_HEIGHT - YOFFSET - i * 2, TFT_WHITE);
}

Paddle paddle1 = Paddle(5, 40, TFT_RED, 10); // width of paddle, height of paddle, color, X offset.
Paddle paddle2 = Paddle(5, 40, TFT_GREEN, 300);
Ball ball = Ball(5, TFT_YELLOW); // Dimension of ball (sq), color
Court court = Court();

void setup() {
  tft.begin();
  tft.setRotation(3);
  spr.createSprite(TFT_WIDTH, TFT_HEIGHT);

  pinMode(WIO_BUZZER, OUTPUT);
  pinMode(UP_BUTTON, INPUT);
  pinMode(DOWN_BUTTON, INPUT);
  pinMode(RESET_BUTTON, INPUT);
  pinMode(SUP_BUTTON, INPUT);
  pinMode(SDOWN_BUTTON, INPUT);
  refresh();
}

void refresh() {
  spr.fillSprite(TFT_BLACK);
  court.draw();
  paddle1.draw();
  paddle2.draw();
  ball.draw();
  spr.pushSprite(0, 0);
}

void loop() {
  if (digitalRead(UP_BUTTON) == LOW) {
    paddle1.moveUp(20);
  } else if (digitalRead(DOWN_BUTTON) == LOW) {
    paddle1.moveDown(20);
  }
  if (digitalRead(SUP_BUTTON) == LOW) {
    paddle2.moveUp(20);
  } else if (digitalRead(SDOWN_BUTTON) == LOW) {
    paddle2.moveDown(20);
  }

  ball.update(); // move the ball

  if (court.collide(ball)) { // Collided with Side walls?
    ball.reset(); // somebody won!
  } else if (ball.collide(paddle1) || ball.collide(paddle2)) {
    ball.bounce();
  }

  refresh();
}
