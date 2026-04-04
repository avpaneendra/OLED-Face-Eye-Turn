#include <SPI.h>
#include <TFT_22_ILI9225.h>
#include <Servo.h>


#include <../fonts/FreeSans12pt7b.h>
#include <../fonts/FreeSans24pt7b.h>
// ─── PIN CONFIG ─────────────────────────
#define TFT_RST A4
#define TFT_RS  A3
#define TFT_CS  A5
#define TFT_SDI A2
#define TFT_CLK A1
#define TFT_LED 7
TFT_22_ILI9225 tft = TFT_22_ILI9225(
                       TFT_RST, TFT_RS, TFT_CS,
                       TFT_SDI, TFT_CLK, TFT_LED
                     );
Servo myServo;

const int obstaclePin = 2;
int flatPos = 90;
int leftPos = 0;
int rightPos = 180;
bool status = false;
// ─── COLORS ─────────────────────────────
#define BLACK 0x0000
#define CYAN  0x07FF   // light blue

int x1 = 40, y1 = 80, x2 = 140, y2 = 80, px = 0, py = 0;
// ─── SETUP ─────────────────────────────

int16_t x = 60, y = 0, w, h;
void eclose()
{
  if (status == true) {
    tft.drawGFXText(x, y, "CLOSE", COLOR_BLUE); // Print string
  }
  Serial.println("CLOSE");


  tft.fillCircle(x1, y1, 30, CYAN );
  // (x=60, y=80, radius=30)
  tft.fillCircle(x2, y2, 30, CYAN);

  if (status == true) {
    tft.drawGFXText(x, y, "CLOSE", COLOR_BLACK); // Print string
  }
  Serial.println("CLOSE");
}

void ebye()
{
  tft.clear();
}
void esad()
{ if (status == true) {
    tft.drawGFXText(x, y, "SAD", COLOR_BLUE); // Print string
  }
  Serial.println("SAD");


  for (int j = 30; j >= 25; j--)
  {
    tft.fillCircle(x1, 110, j, BLACK );
    // (x=60, y=80, radius=30)
    tft.fillCircle(x2, 110, j, BLACK);
    // (x=160, y=80, radius=30)
  }

  tft.fillCircle(x1, 110, 30, BLACK );
  // (x=60, y=80, radius=30)
  tft.fillCircle(x2, 110, 30, BLACK);

  if (status == true) {
    tft.drawGFXText(x, y, "SAD", COLOR_BLACK); // Print string
  }
  Serial.println("SAD");
}

void esmall()
{ if (status == true) {
    tft.drawGFXText(x, y, "SMALL", COLOR_BLUE); // Print string
  }

  Serial.println("SMALL");

  tft.fillCircle(x1, y1, 30, BLACK );
  // (x=60, y=80, radius=30)
  tft.fillCircle(x2, y2, 30, BLACK);
  // (x=160, y=80, radius=30)
  tft.fillCircle(x1 + px, y1 + py, 15, CYAN);
  // draw circle on RIGHT side

  tft.fillCircle(x2 + px, y2 + py, 15, CYAN);
  if (status == true) {
    tft.drawGFXText(x, y, "SMALL", COLOR_BLACK); // Print string
  }
  Serial.println("SMALL");
}

void eangry()
{
  if (status == true) {
    tft.drawGFXText(x, y, "ANGRY", COLOR_BLUE); // Print string
  }
  Serial.println("ANGRY");

  // tft.fillCircle(x1, y1,30, CYAN);
  // (x=60, y=80, radius=30)
  // tft.fillCircle(x2, y2, 30, CYAN);
  // (x=160, y=80, radius=30)
  tft.drawGFXText(30, y + 50, "#", COLOR_RED);
  tft.drawGFXText(130, y + 50, "#", COLOR_RED);

  //tft.drawGFXText(60, y, "#", COLOR_BLACK);
  //tft.drawGFXText(160, y, "#", COLOR_BLACK);
  delay(100);
  if (status == true) {
    tft.drawGFXText(x, y, "ANGRY", COLOR_BLACK); // Print string
  }
  Serial.println("ANGRY");
}

void eopen()
{
  if (status == true) {
    tft.drawGFXText(x, y, "OPEN", COLOR_BLUE); // Print string
  }
  Serial.println("OPEN");

  tft.fillCircle(x1, y1, 30, CYAN);
  // (x=60, y=80, radius=30)
  tft.fillCircle(x2, y2, 30, CYAN);
  // (x=160, y=80, radius=30)

  for (int i = 0; i <= 10; i++)
  {
    tft.fillCircle(x1 + px, y1 + py, i, BLACK);
    // draw circle on RIGHT side


    tft.fillCircle(x2 + px, y2 + py, i, BLACK);
  }
  if (status == true) {
    tft.drawGFXText(x, y, "OPEN", COLOR_BLACK); // Print string
  }
  Serial.println("OPEN");
}

bool flag = false;



void setup() {
  Serial.begin(9600);
  pinMode(obstaclePin, INPUT);

  myServo.attach(6);
  myServo.write(flatPos);
  tft.begin();
  tft.setOrientation(0);  // landscape
  Serial.println("OPEN setup");


  // Draw second string in smaller font
  tft.setGFXFont(&FreeSans12pt7b);  // Change font
  String s2 = "Hello"; // Create string object
  tft.getGFXTextExtent(s2, x, y, &w, &h); // Get string extents
  y += h + 10; // Set y position to string height plus shift down 10 pixels
  tft.drawGFXText(x, y, s2, COLOR_BLUE); // Print string
  delay(1000);
  tft.drawGFXText(x, y, s2, COLOR_BLACK); // Print string

  /*
    eclose();
    eopen();
    esad();
    eclose();
    esmall();

    eopen();*/
  int px = 0;
  int py = 0;
  Serial.println("CLOSE setup");

}
void handleCommand(String cmd) {
  cmd.toLowerCase();

  if (cmd.indexOf("happy") >= 0) eopen();

  else if (cmd.indexOf("sad") >= 0) {
    esad();
  }
  else if (cmd.indexOf("small") >= 0) {
    esmall();
  }
  else if (cmd.indexOf("close") >= 0)  {
    eclose();
  }
  else if ((cmd.indexOf("open") >= 0) || (cmd.indexOf("hi") >= 0)) {
    eopen();
  }
  else if (cmd.indexOf("angry") >= 0) {
    eangry();
  }
  else if (cmd.indexOf("talk") >= 0) {
    eopen();
    esmall();
    eopen();
    esmall();
    eopen();
    esmall();
  }
  else if (cmd.indexOf("bye") >= 0)  {
    eopen();
    eclose();
    ebye();
  }
  else if (cmd.indexOf("o0") >= 0) {
    tft.clear();
    tft.setOrientation(0);
  }
  else if (cmd.indexOf("o1") >= 0) {
    tft.clear();
    tft.setOrientation(1);
  }
  else if (cmd.indexOf("s1") >= 0) {
    status = true;
  }
  else if (cmd.indexOf("s0") >= 0) {
    status = false;
  }
}
void loop() {

  int obstacleState = digitalRead(obstaclePin);
  Serial.print("Obstacle: ");
  Serial.println(obstacleState);
  if (obstacleState == LOW) {
    // Wet
    myServo.write(leftPos);
  }
  else {
    // Dry
    myServo.write(rightPos);
  }

  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    handleCommand(cmd);
  }
  /* tft.setOrientation(1);  // landscape
    tft.clear();
    if(tft.getOrientation()==1)
    {
    if(flag==false)
    {
    x1=60,x2=160,y1=y2=80;
    flag=true;
    }
    }

    tft.setOrientation(0);  // landscape
    tft.clear();
    flag=false;
    x1=40,y1=80,x2=140,y2=80,px=0,py=0;*/
  myServo.write(flatPos);  q
}
