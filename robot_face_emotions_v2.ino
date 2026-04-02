/*
  Robot Face - 9 Emotional Expressions  (v2 – Seamless Ellipse Edition)
  Display : ILI9225 TFT (176 × 220)
  Library : TFT_22_ILI9225

  Key improvements over v1
  ────────────────────────
  • Seamless redrawing: only the dynamic zones (brows, eyes, mouth, label)
    are erased and redrawn – the head/ears/antenna/nose are drawn ONCE in
    setup().  No full tft.clear() between emotions → zero flicker.
  • Elliptical curves: a proper Bresenham midpoint-ellipse engine replaces
    all circle approximations.  Eyes, pupils, and the "O" mouth are true
    filled/outlined ellipses.  Mouth arcs are rendered as upper/lower halves
    of an ellipse instead of parabola pixel-loops.
  • drawEllipseArc() lets individual quadrant combinations be drawn, which
    gives smooth smiles, frowns, and the surprised "O" mouth.
*/

#include "SPI.h"
#include "TFT_22_ILI9225.h"

// ── Pin map ────────────────────────────────────────────────────────────────
#define TFT_RST A4
#define TFT_RS  A3
#define TFT_CS  A5
#define TFT_SDI A2
#define TFT_CLK A1
#define TFT_LED 0

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED);

// ── Palette ────────────────────────────────────────────────────────────────
#define C_BG      COLOR_BLACK
#define C_FACE    0x4C99        // steel-blue robot skin
#define C_OUTLINE COLOR_WHITE
#define C_EYE_W   COLOR_WHITE
#define C_EYE_P   0x001F        // pupil blue
#define C_NOSE    0x7BEF        // light grey
#define C_LIP     COLOR_RED
#define C_SWEAT   0x07FF        // cyan

// ── Screen centre ──────────────────────────────────────────────────────────
#define CX  88
#define CY 110

// ══════════════════════════════════════════════════════════════════════════
//  LOW-LEVEL ELLIPSE ENGINE  (Bresenham midpoint algorithm)
// ══════════════════════════════════════════════════════════════════════════

/*  fillEllipse – solid filled ellipse
    cx,cy : centre   rx : x-radius   ry : y-radius   */
void fillEllipse(int16_t cx, int16_t cy, int16_t rx, int16_t ry, uint16_t color) {
  if (rx == 0 || ry == 0) return;
  int32_t rx2 = (int32_t)rx * rx;
  int32_t ry2 = (int32_t)ry * ry;
  int16_t x = 0, y = ry;
  int32_t px = 0, py = 2 * rx2 * y;

  // Region 1
  int32_t p = (int32_t)ry2 - rx2 * ry + rx2 / 4;
  while (px < py) {
    tft.drawLine(cx - x, cy + y, cx + x, cy + y, color);
    tft.drawLine(cx - x, cy - y, cx + x, cy - y, color);
    x++;
    px += 2 * ry2;
    if (p < 0) { p += ry2 + px; }
    else       { y--; py -= 2 * rx2; p += ry2 + px - py; }
  }
  // Region 2
  p = (int32_t)ry2 * (x + 0) * (x + 0) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
  while (y >= 0) {
    tft.drawLine(cx - x, cy + y, cx + x, cy + y, color);
    tft.drawLine(cx - x, cy - y, cx + x, cy - y, color);
    y--;
    py -= 2 * rx2;
    if (p > 0) { p += rx2 - py; }
    else       { x++; px += 2 * ry2; p += rx2 - py + px; }
  }
}

/*  drawEllipseOutline – 1-pixel outline of a full ellipse  */
void drawEllipseOutline(int16_t cx, int16_t cy, int16_t rx, int16_t ry, uint16_t color) {
  if (rx == 0 || ry == 0) return;
  int32_t rx2 = (int32_t)rx * rx;
  int32_t ry2 = (int32_t)ry * ry;
  int16_t x = 0, y = ry;
  int32_t px = 0, py = 2 * rx2 * y;

  // Region 1
  int32_t p = (int32_t)ry2 - rx2 * ry + rx2 / 4;
  while (px < py) {
    tft.drawPixel(cx + x, cy + y, color); tft.drawPixel(cx - x, cy + y, color);
    tft.drawPixel(cx + x, cy - y, color); tft.drawPixel(cx - x, cy - y, color);
    x++;
    px += 2 * ry2;
    if (p < 0) { p += ry2 + px; }
    else       { y--; py -= 2 * rx2; p += ry2 + px - py; }
  }
  // Region 2
  p = (int32_t)ry2 * x * x + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
  while (y >= 0) {
    tft.drawPixel(cx + x, cy + y, color); tft.drawPixel(cx - x, cy + y, color);
    tft.drawPixel(cx + x, cy - y, color); tft.drawPixel(cx - x, cy - y, color);
    y--;
    py -= 2 * rx2;
    if (p > 0) { p += rx2 - py; }
    else       { x++; px += 2 * ry2; p += rx2 - py + px; }
  }
}

/*  drawEllipseArc – draw only selected quadrant halves (for smiles / frowns)
    halves: bit0 = bottom half (y > cy), bit1 = top half (y < cy)
    Use halves=0x01 for smile arc (bottom), 0x02 for frown arc (top)        */
void drawEllipseArc(int16_t cx, int16_t cy, int16_t rx, int16_t ry,
                    uint8_t halves, uint16_t color) {
  if (rx == 0 || ry == 0) return;
  int32_t rx2 = (int32_t)rx * rx;
  int32_t ry2 = (int32_t)ry * ry;
  int16_t x = 0, y = ry;
  int32_t px = 0, py = 2 * rx2 * y;
  int32_t p = (int32_t)ry2 - rx2 * ry + rx2 / 4;

  auto plot = [&](int16_t dx, int16_t dy) {
    if ((dy > 0 && (halves & 0x01)) || (dy < 0 && (halves & 0x02)) || dy == 0)
      tft.drawPixel(cx + dx, cy + dy, color);
  };

  while (px < py) {
    plot( x,  y); plot(-x,  y);
    plot( x, -y); plot(-x, -y);
    x++; px += 2 * ry2;
    if (p < 0) { p += ry2 + px; }
    else       { y--; py -= 2 * rx2; p += ry2 + px - py; }
  }
  p = (int32_t)ry2 * x * x + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
  while (y >= 0) {
    plot( x,  y); plot(-x,  y);
    plot( x, -y); plot(-x, -y);
    y--; py -= 2 * rx2;
    if (p > 0) { p += rx2 - py; }
    else       { x++; px += 2 * ry2; p += rx2 - py + px; }
  }
}

// ── Thick arc (2-pixel wide) via two ellipses ─────────────────────────────
void drawEllipseArcThick(int16_t cx, int16_t cy, int16_t rx, int16_t ry,
                         uint8_t halves, uint16_t color) {
  drawEllipseArc(cx, cy, rx,   ry,   halves, color);
  drawEllipseArc(cx, cy, rx,   ry+1, halves, color);
  drawEllipseArc(cx, cy, rx+1, ry,   halves, color);
}

// ══════════════════════════════════════════════════════════════════════════
//  STATIC HEAD (drawn once in setup)
// ══════════════════════════════════════════════════════════════════════════
void drawHead() {
  tft.fillRectangle(30, 40, 146, 190, C_FACE);
  tft.drawRectangle(30, 40, 146, 190, C_OUTLINE);
  // Ears
  tft.fillRectangle(18, 70,  29, 110, C_FACE);
  tft.drawRectangle(18, 70,  29, 110, C_OUTLINE);
  tft.fillRectangle(147, 70, 158, 110, C_FACE);
  tft.drawRectangle(147, 70, 158, 110, C_OUTLINE);
  // Antenna
  tft.fillRectangle(84, 28, 92, 40, C_OUTLINE);
  fillEllipse(88, 25, 6, 6, COLOR_YELLOW);  // circle = ellipse with rx==ry
}

void drawNose() {
  tft.drawLine(88, 110, 82, 125, C_NOSE);
  tft.drawLine(88, 110, 94, 125, C_NOSE);
  tft.drawLine(82, 125, 94, 125, C_NOSE);
}

// ══════════════════════════════════════════════════════════════════════════
//  DYNAMIC ZONE ERASERS
//  Each zone is erased to C_FACE before redrawing, instead of full clear()
// ══════════════════════════════════════════════════════════════════════════

// Brow zone: y 60..75, full face width
void eraseBrows() {
  tft.fillRectangle(31, 60, 145, 75, C_FACE);
}

// Eye zone: y 68..102
void eraseEyes() {
  tft.fillRectangle(31, 68, 145, 102, C_FACE);
}

// Mouth zone: y 152..182
void eraseMouth() {
  tft.fillRectangle(31, 152, 145, 182, C_FACE);
}

// Label zone: y 192..210
void eraseLabel() {
  tft.fillRectangle(31, 192, 145, 210, C_FACE);
}

// Sweat-drop zone (corners of eye region)
void eraseSweat() {
  tft.fillRectangle(31, 95, 55,  112, C_FACE);
  tft.fillRectangle(121, 95, 145, 112, C_FACE);
}

// ══════════════════════════════════════════════════════════════════════════
//  ELLIPTICAL EYES
//  erx,ery  = eye ellipse radii   prx,pry = pupil radii
//  lox,loy  = left-pupil offsets  rox,roy = right-pupil offsets
//  closedL / closedR = draw a horizontal line instead of open eye
// ══════════════════════════════════════════════════════════════════════════
void drawEyes(int16_t lx, int16_t ly, int16_t rx, int16_t ry,
              int16_t erx, int16_t ery, int16_t prx, int16_t pry,
              int16_t lox, int16_t loy, int16_t rox, int16_t roy,
              bool closedL, bool closedR) {
  // Left eye
  fillEllipse(lx, ly, erx, ery, C_EYE_W);
  drawEllipseOutline(lx, ly, erx, ery, C_OUTLINE);
  if (closedL) {
    tft.drawLine(lx - erx, ly, lx + erx, ly, C_OUTLINE);
  } else {
    fillEllipse(lx + lox, ly + loy, prx, pry, C_EYE_P);
  }
  // Right eye
  fillEllipse(rx, ry, erx, ery, C_EYE_W);
  drawEllipseOutline(rx, ry, erx, ery, C_OUTLINE);
  if (closedR) {
    tft.drawLine(rx - erx, ry, rx + erx, ry, C_OUTLINE);
  } else {
    fillEllipse(rx + rox, ry + roy, prx, pry, C_EYE_P);
  }
}

// ══════════════════════════════════════════════════════════════════════════
//  EYEBROWS
// ══════════════════════════════════════════════════════════════════════════
void drawBrows(int16_t ly, int16_t ry, int16_t ldx, int16_t rdx, uint16_t color) {
  tft.drawLine(55, ly - ldx, 75, ly + ldx, color);
  tft.drawLine(55, ly - ldx + 1, 75, ly + ldx + 1, color);
  tft.drawLine(101, ry + rdx, 121, ry - rdx, color);
  tft.drawLine(101, ry + rdx + 1, 121, ry - rdx + 1, color);
}

// ══════════════════════════════════════════════════════════════════════════
//  ELLIPTICAL MOUTH SHAPES
// ══════════════════════════════════════════════════════════════════════════

// Happy: lower half of a wide flat ellipse → arc curves upward at ends
// We draw the BOTTOM half (halves=0x01) of an ellipse centred above the
// mouth line, so the arc opens downward like a smile.
void drawMouth_Happy() {
  int16_t mx = CX, my = 157;  // ellipse centre (above actual arc)
  drawEllipseArcThick(mx, my, 28, 12, 0x01, C_LIP);  // bottom half = smile
}

// Sad: UPPER half of the same ellipse, centred below the mouth line
void drawMouth_Sad() {
  int16_t mx = CX, my = 178;
  drawEllipseArcThick(mx, my, 28, 12, 0x02, C_LIP);  // top half = frown
}

// Angry: flat tight line
void drawMouth_Angry() {
  tft.drawLine(CX - 22, 167, CX + 22, 167, C_LIP);
  tft.drawLine(CX - 22, 168, CX + 22, 168, C_LIP);
  tft.drawLine(CX - 22, 169, CX + 22, 169, C_LIP);
}

// Surprised: filled black ellipse with red outline (open mouth "O")
void drawMouth_Surprised() {
  fillEllipse(CX, 168, 13, 10, C_BG);
  drawEllipseOutline(CX, 168, 13, 10, C_LIP);
  drawEllipseOutline(CX, 168, 12,  9, C_LIP);
}

// Scared: jagged wave (pixel art – unchanged from v1 but erased cleanly)
void drawMouth_Scared() {
  for (int x = -24; x <= 24; x += 2) {
    int y = (x % 8 < 4) ? 2 : -2;
    tft.drawPixel(CX + x, 168 + y, C_LIP);
    tft.drawPixel(CX + x, 169 + y, C_LIP);
  }
}

// Disgusted: asymmetric sneer
void drawMouth_Disgusted() {
  tft.drawLine(CX - 24, 162, CX,     170, C_LIP);
  tft.drawLine(CX,      170, CX + 24, 168, C_LIP);
  tft.drawLine(CX - 24, 163, CX,     171, C_LIP);
}

// Sleepy: gentle shallow smile (small ellipse bottom arc) + Zzz
void drawMouth_Sleepy() {
  drawEllipseArcThick(CX, 161, 16, 6, 0x01, C_LIP);
  tft.setFont(Terminal6x8);
  tft.drawText(112, 55, "z", COLOR_CYAN);
  tft.drawText(120, 48, "Z", COLOR_CYAN);
  tft.drawText(130, 40, "Z", COLOR_CYAN);
}

// Excited: wide open toothy grin rectangle
void drawMouth_Excited() {
  tft.fillRectangle(CX - 28, 158, CX + 28, 176, COLOR_WHITE);
  tft.drawRectangle(CX - 28, 158, CX + 28, 176, C_LIP);
  tft.drawLine(CX, 158, CX, 176, C_LIP);  // tooth divider
}

// Neutral: straight flat double-line
void drawMouth_Neutral() {
  tft.drawLine(CX - 20, 167, CX + 20, 167, C_LIP);
  tft.drawLine(CX - 20, 168, CX + 20, 168, C_LIP);
}

// ── Emotion label ─────────────────────────────────────────────────────────
void drawLabel(const char* text, uint16_t color) {
  tft.setFont(Terminal6x8);
  int16_t tx = CX - (strlen(text) * 6) / 2;
  tft.drawText(tx, 196, text, color);
}

// ══════════════════════════════════════════════════════════════════════════
//  MASTER REFRESH  – erase only dynamic zones, then redraw
// ══════════════════════════════════════════════════════════════════════════
void refreshFace() {
  eraseBrows();
  eraseEyes();
  eraseSweat();
  eraseMouth();
  eraseLabel();
  // Erase the Zzz area (only used by SLEEPY but safe to always clear)
  tft.fillRectangle(108, 38, 145, 68, C_FACE);
}

// ══════════════════════════════════════════════════════════════════════════
//  NINE EXPRESSION RENDERERS  (no tft.clear() – only zone erasure)
// ══════════════════════════════════════════════════════════════════════════

void showHappy() {
  refreshFace();
  drawBrows(72, 72, -4, -4, C_OUTLINE);
  //         lx  ly  rx   ry  erx ery prx pry lox loy rox roy  clL   clR
  drawEyes(  65, 85, 111, 85,  14, 11,  6,  5,  0,  2,  0,  2, false, false);
  drawMouth_Happy();
  drawLabel("HAPPY", COLOR_YELLOW);
}

void showSad() {
  refreshFace();
  drawBrows(76, 76, 5, 5, C_OUTLINE);
  drawEyes(65, 85, 111, 85, 14, 11, 6, 5, 0, 2, 0, 2, false, false);
  drawMouth_Sad();
  // Tear drops (elliptical)
  fillEllipse(62, 104, 3, 4, C_SWEAT);
  fillEllipse(108, 104, 3, 4, C_SWEAT);
  drawLabel("SAD", 0x07FF);
}

void showAngry() {
  refreshFace();
  drawBrows(74, 74, 6, 6, COLOR_RED);
  drawEyes(65, 85, 111, 85, 14, 11, 7, 6, 0, 3, 0, 3, false, false);
  tft.drawRectangle(51, 71, 79,  99, COLOR_RED);
  tft.drawRectangle(97, 71, 125, 99, COLOR_RED);
  drawMouth_Angry();
  drawLabel("ANGRY", COLOR_RED);
}

void showSurprised() {
  refreshFace();
  drawBrows(68, 68, -6, -6, C_OUTLINE);
  // Wide eyes: taller ellipse (ery increased)
  drawEyes(65, 85, 111, 85, 16, 14, 7, 6, 0, 0, 0, 0, false, false);
  drawMouth_Surprised();
  drawLabel("SURPRISED", COLOR_YELLOW);
}

void showScared() {
  refreshFace();
  drawBrows(70, 70, -5, -5, COLOR_WHITE);
  drawEyes(65, 85, 111, 85, 16, 14, 5, 4, 3, -3, 3, -3, false, false);
  drawMouth_Scared();
  fillEllipse(40, 80, 3, 4, C_SWEAT);
  fillEllipse(140, 80, 3, 4, C_SWEAT);
  drawLabel("SCARED", 0x07FF);
}

void showDisgusted() {
  refreshFace();
  drawBrows(76, 72, 3, -2, C_OUTLINE);
  drawEyes(65, 85, 111, 85, 14, 11, 6, 5, -2, 4, 0, 2, false, false);
  // Squint left eye: cover lower half with face colour
  tft.fillRectangle(51, 85, 79, 99, C_FACE);
  tft.drawLine(51, 85, 79, 85, C_OUTLINE);
  drawMouth_Disgusted();
  drawLabel("DISGUSTED", COLOR_GREEN);
}

void showSleepy() {
  refreshFace();
  drawBrows(78, 78, 0, 0, C_OUTLINE);
  drawEyes(65, 85, 111, 85, 14, 11, 6, 5, 0, 4, 0, 4, false, false);
  // Drooping lids: cover top half of eyes
  tft.fillRectangle(51,  71, 79,  84, C_FACE);
  tft.fillRectangle(97,  71, 125, 84, C_FACE);
  tft.drawLine(51, 84, 79,  84, C_OUTLINE);
  tft.drawLine(97, 84, 125, 84, C_OUTLINE);
  drawMouth_Sleepy();
  drawLabel("SLEEPY", 0x7BEF);
}

void showExcited() {
  refreshFace();
  drawBrows(68, 68, -7, -7, COLOR_YELLOW);
  drawEyes(65, 85, 111, 85, 14, 11, 8, 7, 0, 0, 0, 0, false, false);
  // Star overlays on eyes
  tft.drawLine(65-10,85, 65+10,85, COLOR_YELLOW);
  tft.drawLine(65,85-10, 65,85+10, COLOR_YELLOW);
  tft.drawLine(65-7,85-7, 65+7,85+7, COLOR_YELLOW);
  tft.drawLine(65-7,85+7, 65+7,85-7, COLOR_YELLOW);
  tft.drawLine(111-10,85, 111+10,85, COLOR_YELLOW);
  tft.drawLine(111,85-10, 111,85+10, COLOR_YELLOW);
  tft.drawLine(111-7,85-7, 111+7,85+7, COLOR_YELLOW);
  tft.drawLine(111-7,85+7, 111+7,85-7, COLOR_YELLOW);
  drawMouth_Excited();
  drawLabel("EXCITED", COLOR_YELLOW);
}

void showNeutral() {
  refreshFace();
  drawBrows(75, 75, 0, 0, C_OUTLINE);
  drawEyes(65, 85, 111, 85, 14, 11, 6, 5, 0, 0, 0, 0, false, false);
  drawMouth_Neutral();
  drawLabel("NEUTRAL", COLOR_WHITE);
}

// ══════════════════════════════════════════════════════════════════════════
//  SETUP & LOOP
// ══════════════════════════════════════════════════════════════════════════
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

  // Draw static elements ONCE
  drawHead();
  drawNose();
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
