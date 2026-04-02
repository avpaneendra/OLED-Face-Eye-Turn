/*
  Robot Face - 9 Emotional Expressions  (v3 – Organ-Local Seamless Redraw)
  Display : ILI9225 TFT (176 × 220)
  Library : TFT_22_ILI9225

  Strategy
  ────────
  • Head / ears / antenna / nose drawn ONCE in setup() – never touched again.
  • Each organ (left-brow, right-brow, left-eye, right-eye, mouth, label,
    sweat-drops) owns its OWN tight bounding-box erase, then redraws itself
    from its centre outward.  No wide horizontal band wipe, no full clear().
  • Ellipse engine (Bresenham midpoint) for all curves: eyes, pupils, arcs,
    tear-drops, sweat-drops, and mouth shapes.
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
#define C_FACE    0x4C99
#define C_OUTLINE COLOR_WHITE
#define C_EYE_W   COLOR_WHITE
#define C_EYE_P   0x001F
#define C_NOSE    0x7BEF
#define C_LIP     COLOR_RED
#define C_SWEAT   0x07FF

// ── Organ anchor points (fixed geometry) ──────────────────────────────────
#define LEX  65          // left  eye centre X
#define REX  111         // right eye centre X
#define EY   85          // both eyes centre Y
#define ERX  14          // eye ellipse X-radius
#define ERY  11          // eye ellipse Y-radius
#define PRX  6           // pupil X-radius
#define PRY  5           // pupil Y-radius

#define LBX1 55          // left  brow start X
#define LBX2 75          // left  brow end   X
#define RBX1 101         // right brow start X
#define RBX2 121         // right brow end   X
#define BROW_PAD 8       // vertical padding around brow erase box

#define MX   88          // mouth centre X
#define MY   168         // mouth centre Y
#define M_RX 28          // mouth bounding X-radius
#define M_RY 14          // mouth bounding Y-radius

#define LBL_Y 196        // label top Y
#define LBL_H 10         // label height

// ══════════════════════════════════════════════════════════════════════════
//  ELLIPSE ENGINE
// ══════════════════════════════════════════════════════════════════════════
void fillEllipse(int16_t cx, int16_t cy, int16_t rx, int16_t ry, uint16_t col) {
  if (!rx || !ry) return;
  int32_t rx2 = (int32_t)rx*rx, ry2 = (int32_t)ry*ry;
  int16_t x = 0, y = ry;
  int32_t px = 0, py = 2*rx2*y;
  int32_t p = ry2 - rx2*ry + rx2/4;
  while (px < py) {
    tft.drawLine(cx-x, cy+y, cx+x, cy+y, col);
    tft.drawLine(cx-x, cy-y, cx+x, cy-y, col);
    x++; px += 2*ry2;
    if (p < 0) { p += ry2+px; }
    else { y--; py -= 2*rx2; p += ry2+px-py; }
  }
  p = ry2*x*x + rx2*(y-1)*(y-1) - rx2*ry2;
  while (y >= 0) {
    tft.drawLine(cx-x, cy+y, cx+x, cy+y, col);
    tft.drawLine(cx-x, cy-y, cx+x, cy-y, col);
    y--; py -= 2*rx2;
    if (p > 0) { p += rx2-py; }
    else { x++; px += 2*ry2; p += rx2-py+px; }
  }
}

void drawEllipseOutline(int16_t cx, int16_t cy, int16_t rx, int16_t ry, uint16_t col) {
  if (!rx || !ry) return;
  int32_t rx2 = (int32_t)rx*rx, ry2 = (int32_t)ry*ry;
  int16_t x = 0, y = ry;
  int32_t px = 0, py = 2*rx2*y;
  int32_t p = ry2 - rx2*ry + rx2/4;
  while (px < py) {
    tft.drawPixel(cx+x,cy+y,col); tft.drawPixel(cx-x,cy+y,col);
    tft.drawPixel(cx+x,cy-y,col); tft.drawPixel(cx-x,cy-y,col);
    x++; px += 2*ry2;
    if (p < 0) { p += ry2+px; }
    else { y--; py -= 2*rx2; p += ry2+px-py; }
  }
  p = ry2*x*x + rx2*(y-1)*(y-1) - rx2*ry2;
  while (y >= 0) {
    tft.drawPixel(cx+x,cy+y,col); tft.drawPixel(cx-x,cy+y,col);
    tft.drawPixel(cx+x,cy-y,col); tft.drawPixel(cx-x,cy-y,col);
    y--; py -= 2*rx2;
    if (p > 0) { p += rx2-py; }
    else { x++; px += 2*ry2; p += rx2-py+px; }
  }
}

// halves: 0x01 = bottom arc (smile), 0x02 = top arc (frown)
void drawEllipseArc(int16_t cx, int16_t cy, int16_t rx, int16_t ry,
                    uint8_t halves, uint16_t col) {
  if (!rx || !ry) return;
  int32_t rx2 = (int32_t)rx*rx, ry2 = (int32_t)ry*ry;
  int16_t x = 0, y = ry;
  int32_t px = 0, py = 2*rx2*y;
  int32_t p = ry2 - rx2*ry + rx2/4;
  auto plot = [&](int16_t dx, int16_t dy) {
    if ((dy > 0 && (halves & 0x01)) || (dy < 0 && (halves & 0x02)) || dy == 0)
      tft.drawPixel(cx+dx, cy+dy, col);
  };
  while (px < py) {
    plot(x,y); plot(-x,y); plot(x,-y); plot(-x,-y);
    x++; px += 2*ry2;
    if (p < 0) { p += ry2+px; }
    else { y--; py -= 2*rx2; p += ry2+px-py; }
  }
  p = ry2*x*x + rx2*(y-1)*(y-1) - rx2*ry2;
  while (y >= 0) {
    plot(x,y); plot(-x,y); plot(x,-y); plot(-x,-y);
    y--; py -= 2*rx2;
    if (p > 0) { p += rx2-py; }
    else { x++; px += 2*ry2; p += rx2-py+px; }
  }
}

// 3-pixel-thick arc
void thickArc(int16_t cx, int16_t cy, int16_t rx, int16_t ry,
              uint8_t halves, uint16_t col) {
  drawEllipseArc(cx, cy, rx,   ry,   halves, col);
  drawEllipseArc(cx, cy, rx,   ry+1, halves, col);
  drawEllipseArc(cx, cy, rx+1, ry,   halves, col);
}

// ══════════════════════════════════════════════════════════════════════════
//  ORGAN-LOCAL ERASER HELPERS
//  Each function fills just the tight box of that organ with C_FACE.
// ══════════════════════════════════════════════════════════════════════════

// Erase one eyebrow by its centre-Y and slope
void eraseBrow(int16_t startX, int16_t endX, int16_t centreY, int16_t slope) {
  int16_t yMin = centreY - abs(slope) - BROW_PAD;
  int16_t yMax = centreY + abs(slope) + BROW_PAD;
  tft.fillRectangle(startX - 2, yMin, endX + 2, yMax, C_FACE);
}

// Erase one eye by its centre
void eraseEye(int16_t cx, int16_t cy) {
  tft.fillRectangle(cx - ERX - 2, cy - ERY - 2,
                    cx + ERX + 2, cy + ERY + 2, C_FACE);
}

// Erase sweat / tear drop at a point
void eraseBlob(int16_t cx, int16_t cy, int16_t r) {
  tft.fillRectangle(cx - r - 1, cy - r - 1,
                    cx + r + 1, cy + r + 1, C_FACE);
}

// Erase mouth box
void eraseMouth() {
  tft.fillRectangle(MX - M_RX - 2, MY - M_RY - 2,
                    MX + M_RX + 2, MY + M_RY + 2, C_FACE);
}

// Erase label row
void eraseLabel() {
  tft.fillRectangle(31, LBL_Y - 1, 145, LBL_Y + LBL_H + 1, C_FACE);
}

// Erase Zzz area (SLEEPY only)
void eraseZzz() {
  tft.fillRectangle(108, 36, 145, 66, C_FACE);
}

// ══════════════════════════════════════════════════════════════════════════
//  STATIC HEAD  (called once in setup)
// ══════════════════════════════════════════════════════════════════════════
void drawHead() {
  tft.fillRectangle(30,  40, 146, 190, C_FACE);
  tft.drawRectangle(30,  40, 146, 190, C_OUTLINE);
  tft.fillRectangle(18,  70,  29, 110, C_FACE);
  tft.drawRectangle(18,  70,  29, 110, C_OUTLINE);
  tft.fillRectangle(147, 70, 158, 110, C_FACE);
  tft.drawRectangle(147, 70, 158, 110, C_OUTLINE);
  tft.fillRectangle(84,  28,  92,  40, C_OUTLINE);
  fillEllipse(88, 25, 6, 6, COLOR_YELLOW);
}

void drawNose() {
  tft.drawLine(88, 110, 82, 125, C_NOSE);
  tft.drawLine(88, 110, 94, 125, C_NOSE);
  tft.drawLine(82, 125, 94, 125, C_NOSE);
}

// ══════════════════════════════════════════════════════════════════════════
//  ORGAN DRAW FUNCTIONS  – each erases its own box first, draws from centre
// ══════════════════════════════════════════════════════════════════════════

// ── Eyebrow ───────────────────────────────────────────────────────────────
// slope > 0 → inner end lower (angry), slope < 0 → inner end higher (happy)
// lSlope / rSlope: positive value, direction encoded by sign
void drawLeftBrow(int16_t centreY, int16_t slope, uint16_t col) {
  eraseBrow(LBX1, LBX2, centreY, slope);
  // left brow: left-end = centreY-slope, right-end = centreY+slope
  tft.drawLine(LBX1, centreY - slope, LBX2, centreY + slope, col);
  tft.drawLine(LBX1, centreY - slope + 1, LBX2, centreY + slope + 1, col);
}

void drawRightBrow(int16_t centreY, int16_t slope, uint16_t col) {
  eraseBrow(RBX1, RBX2, centreY, slope);
  // right brow mirrors: left-end = centreY+slope, right-end = centreY-slope
  tft.drawLine(RBX1, centreY + slope, RBX2, centreY - slope, col);
  tft.drawLine(RBX1, centreY + slope + 1, RBX2, centreY - slope + 1, col);
}

// ── Single eye (with erase) ───────────────────────────────────────────────
void drawOneEye(int16_t cx, int16_t cy,
                int16_t erx, int16_t ery,
                int16_t prx, int16_t pry,
                int16_t pox, int16_t poy,
                bool closed,
                // optional lid fraction: 0=open, >0=cover that many px from top
                int16_t lidPx,
                // optional red-tint rect around eye
                bool angryTint) {
  eraseEye(cx, cy);
  // Eye white (centre outward)
  fillEllipse(cx, cy, erx, ery, C_EYE_W);
  drawEllipseOutline(cx, cy, erx, ery, C_OUTLINE);
  if (closed) {
    tft.drawLine(cx - erx, cy, cx + erx, cy, C_OUTLINE);
  } else {
    fillEllipse(cx + pox, cy + poy, prx, pry, C_EYE_P);
  }
  if (lidPx > 0) {
    // drooping lid: fill top portion from (cy-ery) down lidPx rows
    tft.fillRectangle(cx - erx - 1, cy - ery - 1,
                      cx + erx + 1, cy - ery + lidPx, C_FACE);
    tft.drawLine(cx - erx - 1, cy - ery + lidPx,
                 cx + erx + 1, cy - ery + lidPx, C_OUTLINE);
  }
  if (angryTint) {
    tft.drawRectangle(cx - erx - 2, cy - ery - 2,
                      cx + erx + 2, cy + ery + 2, COLOR_RED);
  }
}

// ── Mouth shapes (each erases own box) ───────────────────────────────────
void drawMouth_Happy() {
  eraseMouth();
  // Bottom-half arc of an ellipse centred ABOVE mouth-line → opens downward
  thickArc(MX, MY - 8, 26, 10, 0x01, C_LIP);
}

void drawMouth_Sad() {
  eraseMouth();
  // Top-half arc centred BELOW mouth-line → opens upward (frown)
  thickArc(MX, MY + 8, 26, 10, 0x02, C_LIP);
}

void drawMouth_Angry() {
  eraseMouth();
  tft.drawLine(MX - 22, MY, MX + 22, MY,     C_LIP);
  tft.drawLine(MX - 22, MY+1, MX + 22, MY+1, C_LIP);
  tft.drawLine(MX - 22, MY+2, MX + 22, MY+2, C_LIP);
}

void drawMouth_Surprised() {
  eraseMouth();
  fillEllipse(MX, MY, 12, 10, C_BG);
  drawEllipseOutline(MX, MY, 12, 10, C_LIP);
  drawEllipseOutline(MX, MY, 11,  9, C_LIP);
}

void drawMouth_Scared() {
  eraseMouth();
  for (int x = -24; x <= 24; x += 2) {
    int y = (x % 8 < 4) ? 2 : -2;
    tft.drawPixel(MX + x, MY + y,     C_LIP);
    tft.drawPixel(MX + x, MY + y + 1, C_LIP);
  }
}

void drawMouth_Disgusted() {
  eraseMouth();
  tft.drawLine(MX-24, MY-6, MX,    MY+2, C_LIP);
  tft.drawLine(MX,    MY+2, MX+24, MY,   C_LIP);
  tft.drawLine(MX-24, MY-5, MX,    MY+3, C_LIP);
}

void drawMouth_Sleepy() {
  eraseMouth();
  thickArc(MX, MY - 4, 16, 6, 0x01, C_LIP);  // small gentle smile
}

void drawMouth_Excited() {
  eraseMouth();
  tft.fillRectangle(MX-28, MY-8, MX+28, MY+8, COLOR_WHITE);
  tft.drawRectangle(MX-28, MY-8, MX+28, MY+8, C_LIP);
  tft.drawLine(MX, MY-8, MX, MY+8, C_LIP);
}

void drawMouth_Neutral() {
  eraseMouth();
  tft.drawLine(MX-20, MY,   MX+20, MY,   C_LIP);
  tft.drawLine(MX-20, MY+1, MX+20, MY+1, C_LIP);
}

// ── Label ─────────────────────────────────────────────────────────────────
void drawLabel(const char* text, uint16_t col) {
  eraseLabel();
  tft.setFont(Terminal6x8);
  int16_t tx = 88 - (strlen(text) * 6) / 2;
  tft.drawText(tx, LBL_Y, text, col);
}

// ── Zzz (SLEEPY extra) ────────────────────────────────────────────────────
void drawZzz() {
  tft.setFont(Terminal6x8);
  tft.drawText(112, 55, "z", COLOR_CYAN);
  tft.drawText(120, 48, "Z", COLOR_CYAN);
  tft.drawText(130, 40, "Z", COLOR_CYAN);
}

// ══════════════════════════════════════════════════════════════════════════
//  EXPRESSION RENDERERS
//  Each one calls only the organs it needs, in order: brow → eye → mouth → label
//  Organs not used by this emotion are erased cleanly (e.g. sweat drops).
// ══════════════════════════════════════════════════════════════════════════

// Shared sweat-drop erase (called by every emotion that doesn't use them)
void clearSweat() {
  eraseBlob(LEX - 3, EY + 19, 5);   // left tear position
  eraseBlob(REX + 3, EY + 19, 5);
  eraseBlob(40,  80, 5);             // left scared sweat
  eraseBlob(140, 80, 5);
}

void showHappy() {
  clearSweat(); eraseZzz();
  drawLeftBrow(72, -4, C_OUTLINE);          // slope=-4 → raised (inner up)
  drawRightBrow(72, -4, C_OUTLINE);
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY,  0, 2, false, 0, false);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY,  0, 2, false, 0, false);
  drawMouth_Happy();
  drawLabel("HAPPY", COLOR_YELLOW);
}

void showSad() {
  clearSweat(); eraseZzz();
  drawLeftBrow(76,  5, C_OUTLINE);          // slope=+5 → inner drops (sad)
  drawRightBrow(76, 5, C_OUTLINE);
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY,  0, 2, false, 0, false);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY,  0, 2, false, 0, false);
  drawMouth_Sad();
  // Tear drops at bottom of each eye, erased individually each transition
  fillEllipse(LEX - 3, EY + 19, 3, 4, C_SWEAT);
  fillEllipse(REX + 3, EY + 19, 3, 4, C_SWEAT);
  drawLabel("SAD", 0x07FF);
}

void showAngry() {
  clearSweat(); eraseZzz();
  drawLeftBrow(74,  6, COLOR_RED);          // inner drops = V-angry
  drawRightBrow(74, 6, COLOR_RED);
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY,  0, 3, false, 0, true);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY,  0, 3, false, 0, true);
  drawMouth_Angry();
  drawLabel("ANGRY", COLOR_RED);
}

void showSurprised() {
  clearSweat(); eraseZzz();
  drawLeftBrow(68, -6, C_OUTLINE);          // high raised
  drawRightBrow(68,-6, C_OUTLINE);
  drawOneEye(LEX, EY, ERX+2, ERY+3, PRX+1, PRY+1, 0, 0, false, 0, false);
  drawOneEye(REX, EY, ERX+2, ERY+3, PRX+1, PRY+1, 0, 0, false, 0, false);
  drawMouth_Surprised();
  drawLabel("SURPRISED", COLOR_YELLOW);
}

void showScared() {
  clearSweat(); eraseZzz();
  drawLeftBrow(70, -5, COLOR_WHITE);
  drawRightBrow(70,-5, COLOR_WHITE);
  drawOneEye(LEX, EY, ERX+2, ERY+3, PRX-1, PRY-1,  3,-3, false, 0, false);
  drawOneEye(REX, EY, ERX+2, ERY+3, PRX-1, PRY-1,  3,-3, false, 0, false);
  drawMouth_Scared();
  fillEllipse(40,  80, 3, 4, C_SWEAT);
  fillEllipse(140, 80, 3, 4, C_SWEAT);
  drawLabel("SCARED", 0x07FF);
}

void showDisgusted() {
  clearSweat(); eraseZzz();
  drawLeftBrow(76,  3, C_OUTLINE);
  drawRightBrow(72,-2, C_OUTLINE);
  // Left eye squinted (lid covers bottom half)
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY, -2, 4, false, ERY, false);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY,  0, 2, false,  0,  false);
  drawMouth_Disgusted();
  drawLabel("DISGUSTED", COLOR_GREEN);
}

void showSleepy() {
  clearSweat();
  drawLeftBrow(78,  0, C_OUTLINE);
  drawRightBrow(78, 0, C_OUTLINE);
  // Both eyes half-drooped: lid covers top half (lidPx = ERY)
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY, 0, 4, false, ERY, false);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY, 0, 4, false, ERY, false);
  drawMouth_Sleepy();
  drawZzz();
  drawLabel("SLEEPY", 0x7BEF);
}

void showExcited() {
  clearSweat(); eraseZzz();
  drawLeftBrow(68, -7, COLOR_YELLOW);
  drawRightBrow(68,-7, COLOR_YELLOW);
  drawOneEye(LEX, EY, ERX, ERY, PRX+2, PRY+2, 0, 0, false, 0, false);
  drawOneEye(REX, EY, ERX, ERY, PRX+2, PRY+2, 0, 0, false, 0, false);
  // Star overlays on each eye
  const int16_t starCX[2] = { LEX, REX };
  for (uint8_t i = 0; i < 2; i++) {
    int16_t cx = starCX[i];
    tft.drawLine(cx-10, EY, cx+10, EY, COLOR_YELLOW);
    tft.drawLine(cx, EY-10, cx, EY+10, COLOR_YELLOW);
    tft.drawLine(cx-7, EY-7, cx+7, EY+7, COLOR_YELLOW);
    tft.drawLine(cx-7, EY+7, cx+7, EY-7, COLOR_YELLOW);
  }
  drawMouth_Excited();
  drawLabel("EXCITED", COLOR_YELLOW);
}

void showNeutral() {
  clearSweat(); eraseZzz();
  drawLeftBrow(75,  0, C_OUTLINE);
  drawRightBrow(75, 0, C_OUTLINE);
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY, 0, 0, false, 0, false);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY, 0, 0, false, 0, false);
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

  drawHead();   // static – never redrawn
  drawNose();   // static – never redrawn
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
