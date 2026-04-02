/*
  Robot Face - 9 Emotional Expressions
  Display: ILI9225 TFT (176x220)
  Library: TFT_22_ILI9225

  Emotions cycle:
  1. HAPPY      2. SAD       3. ANGRY
  4. SURPRISED  5. SCARED    6. DISGUSTED
  7. SLEEPY     8. EXCITED   9. NEUTRAL

  Each expression displays for 1 second.
*/

#include "SPI.h"
#include "TFT_22_ILI9225.h"

#define TFT_RST A4
#define TFT_RS  A3
#define TFT_CS  A5
#define TFT_SDI A2
#define TFT_CLK A1
#define TFT_LED 0

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED);

// ── Palette ────────────────────────────────────────────────────────────────
#define C_BG      COLOR_BLACK
#define C_FACE    0x4C99   // steel-blue robot skin
#define C_OUTLINE COLOR_WHITE
#define C_EYE_W   COLOR_WHITE
#define C_EYE_P   0x001F   // pupil blue
#define C_NOSE    0x7BEF   // light grey
#define C_LIP     COLOR_RED
#define C_SWEAT   0x07FF   // cyan

// Screen centre
#define CX  88
#define CY 110

// ── Helper: draw filled circle (midpoint) ──────────────────────────────────
void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  for (int16_t y = -r; y <= r; y++)
    for (int16_t x = -r; x <= r; x++)
      if (x*x + y*y <= r*r)
        tft.drawPixel(x0+x, y0+y, color);
}

// ── Helper: draw circle outline ───────────────────────────────────────────
void drawCircleOutline(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  int16_t f = 1-r, ddF_x = 1, ddF_y = -2*r, x = 0, y = r;
  tft.drawPixel(x0, y0+r, color); tft.drawPixel(x0, y0-r, color);
  tft.drawPixel(x0+r, y0, color); tft.drawPixel(x0-r, y0, color);
  while (x < y) {
    if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
    x++; ddF_x += 2; f += ddF_x;
    tft.drawPixel(x0+x, y0+y, color); tft.drawPixel(x0-x, y0+y, color);
    tft.drawPixel(x0+x, y0-y, color); tft.drawPixel(x0-x, y0-y, color);
    tft.drawPixel(x0+y, y0+x, color); tft.drawPixel(x0-y, y0+x, color);
    tft.drawPixel(x0+y, y0-x, color); tft.drawPixel(x0-y, y0-x, color);
  }
}

// ── Helper: draw filled rectangle ─────────────────────────────────────────
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  tft.fillRectangle(x, y, x+w-1, y+h-1, color);
}

// ── Draw robot head & base face elements ──────────────────────────────────
void drawHead() {
  // Outer rounded rectangle (head)
  tft.fillRectangle(30, 40, 146, 190, C_FACE);
  tft.drawRectangle(30, 40, 146, 190, C_OUTLINE);
  // Ears (side panels)
  tft.fillRectangle(18, 70, 29, 110, C_FACE);
  tft.drawRectangle(18, 70, 29, 110, C_OUTLINE);
  tft.fillRectangle(147, 70, 158, 110, C_FACE);
  tft.drawRectangle(147, 70, 158, 110, C_OUTLINE);
  // Antenna
  tft.fillRectangle(84, 28, 92, 40, C_OUTLINE);
  fillCircle(88, 25, 6, COLOR_YELLOW);
}

// ── Draw nose (always same: small triangle) ───────────────────────────────
void drawNose() {
  // Simple triangle nose
  tft.drawLine(88, 110, 82, 125, C_NOSE);
  tft.drawLine(88, 110, 94, 125, C_NOSE);
  tft.drawLine(82, 125, 94, 125, C_NOSE);
}

// ── Draw eye whites + pupils ──────────────────────────────────────────────
//   lx,ly = left eye centre   rx,ry = right eye centre
//   er = eye white radius     pr = pupil radius
//   lox,loy = left pupil offset from centre (same for right = rox,roy)
void drawEyes(int16_t lx, int16_t ly, int16_t rx, int16_t ry,
              int16_t er, int16_t pr,
              int16_t lox, int16_t loy,
              int16_t rox, int16_t roy,
              bool closedL, bool closedR) {
  // Left eye
  fillCircle(lx, ly, er, C_EYE_W);
  drawCircleOutline(lx, ly, er, C_OUTLINE);
  if (closedL) {
    tft.drawLine(lx-er, ly, lx+er, ly, C_OUTLINE); // closed = line
  } else {
    fillCircle(lx+lox, ly+loy, pr, C_EYE_P);
  }
  // Right eye
  fillCircle(rx, ry, er, C_EYE_W);
  drawCircleOutline(rx, ry, er, C_OUTLINE);
  if (closedR) {
    tft.drawLine(rx-er, ry, rx+er, ry, C_OUTLINE);
  } else {
    fillCircle(rx+rox, ry+roy, pr, C_EYE_P);
  }
}

// ── Draw eyebrows ─────────────────────────────────────────────────────────
//   Positive dy = brow tilts down toward centre (angry)
//   Negative dy = brow tilts up toward centre (sad)
void drawBrows(int16_t ly, int16_t ry, int16_t ldx, int16_t rdx, uint16_t color) {
  // left brow: from (55,ly-ldx) to (75,ly+ldx)
  tft.drawLine(55, ly-ldx, 75, ly+ldx, color);
  tft.drawLine(55, ly-ldx+1, 75, ly+ldx+1, color);
  // right brow
  tft.drawLine(101, ry+rdx, 121, ry-rdx, color);
  tft.drawLine(101, ry+rdx+1, 121, ry-rdx+1, color);
}

// ── Draw mouth shapes ─────────────────────────────────────────────────────
void drawMouth_Happy() {
  // Big arc smile
  for (int x = -28; x <= 28; x++) {
    int y = (x*x)/28 - 8;
    tft.drawPixel(CX+x, 162+y, C_LIP);
    tft.drawPixel(CX+x, 163+y, C_LIP);
  }
}

void drawMouth_Sad() {
  // Inverted arc frown
  for (int x = -28; x <= 28; x++) {
    int y = -(x*x)/28 + 8;
    tft.drawPixel(CX+x, 168+y, C_LIP);
    tft.drawPixel(CX+x, 169+y, C_LIP);
  }
}

void drawMouth_Angry() {
  // Flat tight line
  tft.drawLine(CX-22, 168, CX+22, 168, C_LIP);
  tft.drawLine(CX-22, 169, CX+22, 169, C_LIP);
}

void drawMouth_Surprised() {
  // Big open oval "O"
  fillCircle(CX, 168, 12, COLOR_BLACK);
  drawCircleOutline(CX, 168, 12, C_LIP);
  drawCircleOutline(CX, 168, 11, C_LIP);
}

void drawMouth_Scared() {
  // Trembling wave
  for (int x = -24; x <= 24; x += 2) {
    int y = (x % 8 < 4) ? 2 : -2;
    tft.drawPixel(CX+x, 168+y, C_LIP);
    tft.drawPixel(CX+x, 169+y, C_LIP);
  }
}

void drawMouth_Disgusted() {
  // Asymmetric sneer (left side raised)
  tft.drawLine(CX-24, 162, CX,    170, C_LIP);
  tft.drawLine(CX,    170, CX+24, 168, C_LIP);
  tft.drawLine(CX-24, 163, CX,    171, C_LIP);
}

void drawMouth_Sleepy() {
  // Small gentle smile with Zzz
  for (int x = -16; x <= 16; x++) {
    int y = (x*x)/28;
    tft.drawPixel(CX+x, 165+y, C_LIP);
  }
  // Zzz text
  tft.setFont(Terminal6x8);
  tft.drawText(112, 55, "z", COLOR_CYAN);
  tft.drawText(120, 48, "Z", COLOR_CYAN);
  tft.drawText(130, 40, "Z", COLOR_CYAN);
}

void drawMouth_Excited() {
  // Wide open toothy grin
  tft.fillRectangle(CX-28, 158, CX+28, 176, COLOR_WHITE);
  tft.drawRectangle(CX-28, 158, CX+28, 176, C_LIP);
  // Teeth divider
  tft.drawLine(CX, 158, CX, 176, C_LIP);
}

void drawMouth_Neutral() {
  // Straight flat line
  tft.drawLine(CX-20, 168, CX+20, 168, C_LIP);
  tft.drawLine(CX-20, 169, CX+20, 169, C_LIP);
}

// ── Draw emotion label ─────────────────────────────────────────────────────
void drawLabel(const char* text, uint16_t color) {
  tft.setFont(Terminal6x8);
  // Centre-ish the text (each char = 6px wide)
  int len = strlen(text);
  int16_t tx = CX - (len * 6) / 2;
  tft.drawText(tx, 196, text, color);
}

// ═══════════════════════════════════════════════════════════════════════════
// Nine expression renderers
// ═══════════════════════════════════════════════════════════════════════════

void showHappy() {
  tft.clear();
  drawHead();
  drawBrows(72, 72, -4, -4, C_OUTLINE);           // raised brows
  drawEyes(65,85, 111,85, 14,6, 0,2, 0,2, false,false);
  drawNose();
  drawMouth_Happy();
  drawLabel("HAPPY", COLOR_YELLOW);
}

void showSad() {
  tft.clear();
  drawHead();
  drawBrows(76, 76, 5, 5, C_OUTLINE);             // angled down inner
  drawEyes(65,85, 111,85, 14,6, 0,2, 0,2, false,false);
  drawNose();
  drawMouth_Sad();
  // Tear drops
  fillCircle(62, 105, 3, C_SWEAT);
  fillCircle(108, 105, 3, C_SWEAT);
  drawLabel("SAD", 0x07FF);
}

void showAngry() {
  tft.clear();
  drawHead();
  drawBrows(74, 74, 6, 6, COLOR_RED);             // V-shape angry brows
  drawEyes(65,85, 111,85, 14,7, 0,3, 0,3, false,false);
  // Red tint over eyes
  tft.drawRectangle(51,71, 79,99, COLOR_RED);
  tft.drawRectangle(97,71, 125,99, COLOR_RED);
  drawNose();
  drawMouth_Angry();
  drawLabel("ANGRY", COLOR_RED);
}

void showSurprised() {
  tft.clear();
  drawHead();
  drawBrows(68, 68, -6, -6, C_OUTLINE);           // high raised brows
  drawEyes(65,85, 111,85, 16,7, 0,0, 0,0, false,false);
  drawNose();
  drawMouth_Surprised();
  drawLabel("SURPRISED", COLOR_YELLOW);
}

void showScared() {
  tft.clear();
  drawHead();
  drawBrows(70, 70, -5, -5, COLOR_WHITE);
  // Wide open eyes with small pupils (looking up-right)
  drawEyes(65,85, 111,85, 16,5, 3,-3, 3,-3, false,false);
  drawNose();
  drawMouth_Scared();
  // Sweat drops
  fillCircle(40, 80, 3, C_SWEAT);
  fillCircle(140, 80, 3, C_SWEAT);
  drawLabel("SCARED", 0x07FF);
}

void showDisgusted() {
  tft.clear();
  drawHead();
  drawBrows(76, 72, 3, -2, C_OUTLINE);            // asymmetric
  // One squinted eye
  drawEyes(65,85, 111,85, 14,6, -2,4, 0,2, false,false);
  // Squint left eye
  tft.fillRectangle(51, 85, 79, 99, C_FACE);
  tft.drawLine(51,85, 79,85, C_OUTLINE);
  drawNose();
  drawMouth_Disgusted();
  drawLabel("DISGUSTED", COLOR_GREEN);
}

void showSleepy() {
  tft.clear();
  drawHead();
  drawBrows(78, 78, 0, 0, C_OUTLINE);
  // Half-closed eyes
  drawEyes(65,85, 111,85, 14,6, 0,4, 0,4, false,false);
  // Cover top half of eyes (drooping lids)
  tft.fillRectangle(51,71, 79,84, C_FACE);
  tft.fillRectangle(97,71, 125,84, C_FACE);
  tft.drawLine(51,84, 79,84, C_OUTLINE);
  tft.drawLine(97,84, 125,84, C_OUTLINE);
  drawNose();
  drawMouth_Sleepy();
  drawLabel("SLEEPY", 0x7BEF);
}

void showExcited() {
  tft.clear();
  drawHead();
  drawBrows(68, 68, -7, -7, COLOR_YELLOW);
  // Stars for eyes instead of circles
  drawEyes(65,85, 111,85, 14,8, 0,0, 0,0, false,false);
  // Star overlays
  tft.drawLine(65-10,85, 65+10,85, COLOR_YELLOW);
  tft.drawLine(65,85-10, 65,85+10, COLOR_YELLOW);
  tft.drawLine(65-7,85-7, 65+7,85+7, COLOR_YELLOW);
  tft.drawLine(65-7,85+7, 65+7,85-7, COLOR_YELLOW);
  tft.drawLine(111-10,85, 111+10,85, COLOR_YELLOW);
  tft.drawLine(111,85-10, 111,85+10, COLOR_YELLOW);
  tft.drawLine(111-7,85-7, 111+7,85+7, COLOR_YELLOW);
  tft.drawLine(111-7,85+7, 111+7,85-7, COLOR_YELLOW);
  drawNose();
  drawMouth_Excited();
  drawLabel("EXCITED", COLOR_YELLOW);
}

void showNeutral() {
  tft.clear();
  drawHead();
  drawBrows(75, 75, 0, 0, C_OUTLINE);             // flat brows
  drawEyes(65,85, 111,85, 14,6, 0,0, 0,0, false,false);
  drawNose();
  drawMouth_Neutral();
  drawLabel("NEUTRAL", COLOR_WHITE);
}

// ═══════════════════════════════════════════════════════════════════════════

void setup() {
#if defined(ESP32)
  hspi.begin();
  tft.begin(hspi);
#else
  tft.begin();
#endif
  tft.setOrientation(0);
  tft.setBacklight(true);
  tft.clear();
}

void loop() {
  showHappy();      delay(1000);
  showSad();        delay(1000);
  showAngry();      delay(1000);
  showSurprised();  delay(1000);
  showScared();     delay(1000);
  showDisgusted();  delay(1000);
  showSleepy();     delay(1000);
  showExcited();    delay(1000);
  showNeutral();    delay(1000);
}
