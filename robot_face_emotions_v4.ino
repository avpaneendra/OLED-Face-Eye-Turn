/*
  Robot Face - 9 Emotional Expressions  (v4 – Full-Screen + Inset Ears)
  Display : ILI9225 TFT (176 × 220)
  Library : TFT_22_ILI9225

  Layout (pixels)
  ───────────────
  Screen          : 0,0 → 175,219   (176 wide × 220 tall)
  Head fill       : 0,0 → 175,219   (entire screen = face)
  Inset ear L     : 4,80  → 22,150  (inside face, left side)
  Inset ear R     : 153,80 → 171,150 (inside face, right side)
  Antenna ball    : cx=87, cy=0, r=8
  Antenna stem    : x 83–91, y 0–14

  Eye centres     : L=(55,90)  R=(121,90)
  Eye radii       : rx=19, ry=15
  Pupil radii     : rx=8,  ry=7
  Brows           : L x 36–74,  R x 102–140,  y ~68
  Nose            : triangle centred x=88, y 125–148
  Mouth           : cx=88, cy=178, rx=34, ry=16
  Label           : y=207  (bottom strip)
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
#define C_EAR     0x3A77        // slightly darker than face
#define C_EYE_W   COLOR_WHITE
#define C_EYE_P   0x001F
#define C_NOSE    0x7BEF
#define C_LIP     COLOR_RED
#define C_SWEAT   0x07FF

// ── Fixed geometry ─────────────────────────────────────────────────────────
// Eyes
#define LEX   55           // left  eye X
#define REX  121           // right eye X
#define EY    90           // eye Y
#define ERX   19           // eye ellipse X-radius
#define ERY   15           // eye ellipse Y-radius
#define PRX    8           // pupil X-radius
#define PRY    7           // pupil Y-radius

// Brows  (span 38 px wide, centred over each eye)
#define LBX1  36           // left  brow start X
#define LBX2  74           // left  brow end   X
#define RBX1 102           // right brow start X
#define RBX2 140           // right brow end   X
#define BROW_CY 67         // default brow centre Y
#define BROW_PAD 10

// Nose triangle
#define NTX   88           // nose tip X
#define NTY  125           // nose tip Y
#define NBY  148           // nose base Y
#define NBL   80           // nose base left  X
#define NBR   96           // nose base right X

// Mouth
#define MX    88           // mouth centre X
#define MY   178           // mouth centre Y
#define M_RX  34           // bounding X-radius
#define M_RY  16           // bounding Y-radius

// Label
#define LBL_Y 207
#define LBL_H  10

// Inset ears (inside face)
#define EAR_LX1   4
#define EAR_LX2  22
#define EAR_RX1 153
#define EAR_RX2 171
#define EAR_Y1   80
#define EAR_Y2  150

// Sweat blobs
#define SW_L_X  (LEX - 4)
#define SW_L_Y  (EY + 22)
#define SW_R_X  (REX + 4)
#define SW_R_Y  (EY + 22)
#define SC_L_X  25
#define SC_L_Y  85
#define SC_R_X 151
#define SC_R_Y  85

// ══════════════════════════════════════════════════════════════════════════
//  ELLIPSE ENGINE  (Bresenham midpoint – no lambdas, AVR-safe)
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

// Arc plotter used internally – no lambda, plain function
static uint8_t _arcHalves;
static uint16_t _arcCol;
static int16_t  _arcCX, _arcCY;
void _arcPlot(int16_t dx, int16_t dy) {
  if ((dy > 0 && (_arcHalves & 0x01)) ||
      (dy < 0 && (_arcHalves & 0x02)) ||
       dy == 0)
    tft.drawPixel(_arcCX + dx, _arcCY + dy, _arcCol);
}

// halves: 0x01 = bottom (smile), 0x02 = top (frown)
void drawEllipseArc(int16_t cx, int16_t cy, int16_t rx, int16_t ry,
                    uint8_t halves, uint16_t col) {
  if (!rx || !ry) return;
  _arcCX = cx; _arcCY = cy; _arcHalves = halves; _arcCol = col;
  int32_t rx2 = (int32_t)rx*rx, ry2 = (int32_t)ry*ry;
  int16_t x = 0, y = ry;
  int32_t px = 0, py = 2*rx2*y;
  int32_t p = ry2 - rx2*ry + rx2/4;
  while (px < py) {
    _arcPlot(x,y); _arcPlot(-x,y); _arcPlot(x,-y); _arcPlot(-x,-y);
    x++; px += 2*ry2;
    if (p < 0) { p += ry2+px; }
    else { y--; py -= 2*rx2; p += ry2+px-py; }
  }
  p = ry2*x*x + rx2*(y-1)*(y-1) - rx2*ry2;
  while (y >= 0) {
    _arcPlot(x,y); _arcPlot(-x,y); _arcPlot(x,-y); _arcPlot(-x,-y);
    y--; py -= 2*rx2;
    if (p > 0) { p += rx2-py; }
    else { x++; px += 2*ry2; p += rx2-py+px; }
  }
}

void thickArc(int16_t cx, int16_t cy, int16_t rx, int16_t ry,
              uint8_t halves, uint16_t col) {
  drawEllipseArc(cx, cy, rx,   ry,   halves, col);
  drawEllipseArc(cx, cy, rx,   ry+1, halves, col);
  drawEllipseArc(cx, cy, rx+1, ry,   halves, col);
  drawEllipseArc(cx, cy, rx+1, ry+1, halves, col);
}

// ══════════════════════════════════════════════════════════════════════════
//  ORGAN-LOCAL ERASERS  – fill tight bounding box with C_FACE
// ══════════════════════════════════════════════════════════════════════════
void eraseBrow(int16_t x1, int16_t x2, int16_t cy, int16_t slope) {
  int16_t pad = abs(slope) + BROW_PAD;
  tft.fillRectangle(x1-2, cy-pad, x2+2, cy+pad, C_FACE);
}

void eraseEye(int16_t cx, int16_t cy, int16_t erx, int16_t ery) {
  tft.fillRectangle(cx-erx-2, cy-ery-2, cx+erx+2, cy+ery+2, C_FACE);
}

void eraseBlob(int16_t cx, int16_t cy, int16_t r) {
  tft.fillRectangle(cx-r-1, cy-r-1, cx+r+1, cy+r+1, C_FACE);
}

void eraseMouth() {
  tft.fillRectangle(MX-M_RX-2, MY-M_RY-2, MX+M_RX+2, MY+M_RY+2, C_FACE);
}

void eraseLabel() {
  tft.fillRectangle(1, LBL_Y-1, 174, LBL_Y+LBL_H+1, C_FACE);
}

void eraseZzz() {
  tft.fillRectangle(130, 30, 175, 75, C_FACE);
}

// ══════════════════════════════════════════════════════════════════════════
//  STATIC ELEMENTS  (drawn once in setup, never touched again)
// ══════════════════════════════════════════════════════════════════════════
void drawHead() {
  // Full-screen face fill
  tft.fillRectangle(0, 0, 175, 219, C_FACE);
  // Thin border so the face has an edge on screen
  tft.drawRectangle(0, 0, 175, 219, C_OUTLINE);

  // Inset ears – filled with darker colour, outlined
  tft.fillRectangle(EAR_LX1, EAR_Y1, EAR_LX2, EAR_Y2, C_EAR);
  tft.drawRectangle(EAR_LX1, EAR_Y1, EAR_LX2, EAR_Y2, C_OUTLINE);
  // Ear detail (horizontal line at mid)
  int16_t earMidY = (EAR_Y1 + EAR_Y2) / 2;
  tft.drawLine(EAR_LX1+1, earMidY, EAR_LX2-1, earMidY, C_OUTLINE);

  tft.fillRectangle(EAR_RX1, EAR_Y1, EAR_RX2, EAR_Y2, C_EAR);
  tft.drawRectangle(EAR_RX1, EAR_Y1, EAR_RX2, EAR_Y2, C_OUTLINE);
  tft.drawLine(EAR_RX1+1, earMidY, EAR_RX2-1, earMidY, C_OUTLINE);

  // Antenna stem + ball
  tft.fillRectangle(84, 8, 92, 22, C_OUTLINE);
  fillEllipse(88, 5, 8, 8, COLOR_YELLOW);
  drawEllipseOutline(88, 5, 8, 8, C_OUTLINE);
}

void drawNose() {
  // Filled triangle: tip at (NTX,NTY), base at (NBL,NBY)–(NBR,NBY)
  for (int16_t y = NTY; y <= NBY; y++) {
    float t = (float)(y - NTY) / (NBY - NTY);
    int16_t xl = NTX - (int16_t)(t * (NTX - NBL));
    int16_t xr = NTX + (int16_t)(t * (NBR - NTX));
    tft.drawLine(xl, y, xr, y, C_NOSE);
  }
  tft.drawLine(NTX, NTY, NBL, NBY, C_OUTLINE);
  tft.drawLine(NTX, NTY, NBR, NBY, C_OUTLINE);
  tft.drawLine(NBL, NBY, NBR, NBY, C_OUTLINE);
}

// ══════════════════════════════════════════════════════════════════════════
//  ORGAN DRAW FUNCTIONS
// ══════════════════════════════════════════════════════════════════════════

// ── Brows ─────────────────────────────────────────────────────────────────
void drawLeftBrow(int16_t cy, int16_t slope, uint16_t col) {
  eraseBrow(LBX1, LBX2, cy, slope);
  tft.drawLine(LBX1, cy-slope, LBX2, cy+slope, col);
  tft.drawLine(LBX1, cy-slope+1, LBX2, cy+slope+1, col);
  tft.drawLine(LBX1, cy-slope+2, LBX2, cy+slope+2, col);
}

void drawRightBrow(int16_t cy, int16_t slope, uint16_t col) {
  eraseBrow(RBX1, RBX2, cy, slope);
  tft.drawLine(RBX1, cy+slope, RBX2, cy-slope, col);
  tft.drawLine(RBX1, cy+slope+1, RBX2, cy-slope+1, col);
  tft.drawLine(RBX1, cy+slope+2, RBX2, cy-slope+2, col);
}

// ── Eye ───────────────────────────────────────────────────────────────────
//  lidPx  : pixels to cover from the TOP of the eye (drooping lid)
//  angryTint : draw red rect around eye
void drawOneEye(int16_t cx, int16_t cy,
                int16_t erx, int16_t ery,
                int16_t prx, int16_t pry,
                int16_t pox, int16_t poy,
                bool closed, int16_t lidPx, bool angryTint) {
  eraseEye(cx, cy, erx, ery);
  fillEllipse(cx, cy, erx, ery, C_EYE_W);
  drawEllipseOutline(cx, cy, erx, ery, C_OUTLINE);
  if (closed) {
    tft.drawLine(cx-erx, cy, cx+erx, cy, C_OUTLINE);
    tft.drawLine(cx-erx, cy+1, cx+erx, cy+1, C_OUTLINE);
  } else {
    fillEllipse(cx+pox, cy+poy, prx, pry, C_EYE_P);
    drawEllipseOutline(cx+pox, cy+poy, prx, pry, C_OUTLINE);
  }
  if (lidPx > 0) {
    tft.fillRectangle(cx-erx-1, cy-ery-1, cx+erx+1, cy-ery+lidPx, C_FACE);
    tft.drawLine(cx-erx-1, cy-ery+lidPx, cx+erx+1, cy-ery+lidPx, C_OUTLINE);
  }
  if (angryTint) {
    tft.drawRectangle(cx-erx-3, cy-ery-3, cx+erx+3, cy+ery+3, COLOR_RED);
    tft.drawRectangle(cx-erx-2, cy-ery-2, cx+erx+2, cy+ery+2, COLOR_RED);
  }
}

// ── Mouths ────────────────────────────────────────────────────────────────
void drawMouth_Happy() {
  eraseMouth();
  thickArc(MX, MY-10, M_RX, 12, 0x01, C_LIP);
}

void drawMouth_Sad() {
  eraseMouth();
  thickArc(MX, MY+10, M_RX, 12, 0x02, C_LIP);
}

void drawMouth_Angry() {
  eraseMouth();
  for (int16_t i = 0; i < 3; i++)
    tft.drawLine(MX-28, MY+i, MX+28, MY+i, C_LIP);
}

void drawMouth_Surprised() {
  eraseMouth();
  fillEllipse(MX, MY, 16, 13, C_BG);
  drawEllipseOutline(MX, MY, 16, 13, C_LIP);
  drawEllipseOutline(MX, MY, 15, 12, C_LIP);
}

void drawMouth_Scared() {
  eraseMouth();
  for (int16_t x = -30; x <= 30; x += 2) {
    int16_t y = (((x + 30) / 6) % 2 == 0) ? 3 : -3;
    tft.drawPixel(MX+x, MY+y,   C_LIP);
    tft.drawPixel(MX+x, MY+y+1, C_LIP);
    tft.drawPixel(MX+x, MY+y+2, C_LIP);
  }
}

void drawMouth_Disgusted() {
  eraseMouth();
  tft.drawLine(MX-30, MY-8, MX,    MY+4, C_LIP);
  tft.drawLine(MX,    MY+4, MX+30, MY,   C_LIP);
  tft.drawLine(MX-30, MY-7, MX,    MY+5, C_LIP);
  tft.drawLine(MX,    MY+5, MX+30, MY+1, C_LIP);
}

void drawMouth_Sleepy() {
  eraseMouth();
  thickArc(MX, MY-5, 20, 8, 0x01, C_LIP);
}

void drawMouth_Excited() {
  eraseMouth();
  tft.fillRectangle(MX-M_RX, MY-M_RY+2, MX+M_RX, MY+M_RY-2, COLOR_WHITE);
  tft.drawRectangle(MX-M_RX, MY-M_RY+2, MX+M_RX, MY+M_RY-2, C_LIP);
  tft.drawLine(MX, MY-M_RY+2, MX, MY+M_RY-2, C_LIP);
}

void drawMouth_Neutral() {
  eraseMouth();
  for (int16_t i = 0; i < 3; i++)
    tft.drawLine(MX-24, MY+i, MX+24, MY+i, C_LIP);
}

// ── Label ─────────────────────────────────────────────────────────────────
void drawLabel(const char* text, uint16_t col) {
  eraseLabel();
  tft.setFont(Terminal6x8);
  int16_t tx = 88 - ((int16_t)strlen(text) * 6) / 2;
  tft.drawText(tx, LBL_Y, text, col);
}

// ── Zzz ───────────────────────────────────────────────────────────────────
void drawZzz() {
  tft.setFont(Terminal6x8);
  tft.drawText(138, 60, "z", COLOR_CYAN);
  tft.drawText(150, 48, "Z", COLOR_CYAN);
  tft.drawText(163, 35, "Z", COLOR_CYAN);
}

// ══════════════════════════════════════════════════════════════════════════
//  EXPRESSION RENDERERS
// ══════════════════════════════════════════════════════════════════════════

void clearSweat() {
  eraseBlob(SW_L_X, SW_L_Y, 7);
  eraseBlob(SW_R_X, SW_R_Y, 7);
  eraseBlob(SC_L_X, SC_L_Y, 6);
  eraseBlob(SC_R_X, SC_R_Y, 6);
}

void showHappy() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY-4, -5, C_OUTLINE);
  drawRightBrow(BROW_CY-4, -5, C_OUTLINE);
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY,  0, 3, false, 0, false);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY,  0, 3, false, 0, false);
  drawMouth_Happy();
  drawLabel("HAPPY", COLOR_YELLOW);
}

void showSad() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY+3,  6, C_OUTLINE);
  drawRightBrow(BROW_CY+3, 6, C_OUTLINE);
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY,  0, 3, false, 0, false);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY,  0, 3, false, 0, false);
  drawMouth_Sad();
  fillEllipse(SW_L_X, SW_L_Y, 4, 5, C_SWEAT);
  fillEllipse(SW_R_X, SW_R_Y, 4, 5, C_SWEAT);
  drawLabel("SAD", 0x07FF);
}

void showAngry() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY+2,  7, COLOR_RED);
  drawRightBrow(BROW_CY+2, 7, COLOR_RED);
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY,  0, 4, false, 0, true);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY,  0, 4, false, 0, true);
  drawMouth_Angry();
  drawLabel("ANGRY", COLOR_RED);
}

void showSurprised() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY-8, -7, C_OUTLINE);
  drawRightBrow(BROW_CY-8,-7, C_OUTLINE);
  drawOneEye(LEX, EY, ERX+3, ERY+4, PRX+1, PRY+1, 0, 0, false, 0, false);
  drawOneEye(REX, EY, ERX+3, ERY+4, PRX+1, PRY+1, 0, 0, false, 0, false);
  drawMouth_Surprised();
  drawLabel("SURPRISED", COLOR_YELLOW);
}

void showScared() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY-6, -6, C_OUTLINE);
  drawRightBrow(BROW_CY-6,-6, C_OUTLINE);
  drawOneEye(LEX, EY, ERX+3, ERY+4, PRX-1, PRY-1,  4,-4, false, 0, false);
  drawOneEye(REX, EY, ERX+3, ERY+4, PRX-1, PRY-1,  4,-4, false, 0, false);
  drawMouth_Scared();
  fillEllipse(SC_L_X, SC_L_Y, 4, 5, C_SWEAT);
  fillEllipse(SC_R_X, SC_R_Y, 4, 5, C_SWEAT);
  drawLabel("SCARED", 0x07FF);
}

void showDisgusted() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY+3,  4, C_OUTLINE);
  drawRightBrow(BROW_CY-1,-3, C_OUTLINE);
  // Left eye squinted (lid = full ERY to cover top half)
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY, -3, 5, false, ERY, false);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY,  0, 2, false,  0,  false);
  drawMouth_Disgusted();
  drawLabel("DISGUSTED", COLOR_GREEN);
}

void showSleepy() {
  clearSweat();
  drawLeftBrow(BROW_CY+2,  0, C_OUTLINE);
  drawRightBrow(BROW_CY+2, 0, C_OUTLINE);
  drawOneEye(LEX, EY, ERX, ERY, PRX, PRY, 0, 5, false, ERY, false);
  drawOneEye(REX, EY, ERX, ERY, PRX, PRY, 0, 5, false, ERY, false);
  drawMouth_Sleepy();
  drawZzz();
  drawLabel("SLEEPY", 0x7BEF);
}

void showExcited() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY-6, -8, COLOR_YELLOW);
  drawRightBrow(BROW_CY-6,-8, COLOR_YELLOW);
  drawOneEye(LEX, EY, ERX, ERY, PRX+3, PRY+3, 0, 0, false, 0, false);
  drawOneEye(REX, EY, ERX, ERY, PRX+3, PRY+3, 0, 0, false, 0, false);
  // Star overlays
  const int16_t starCX[2] = { LEX, REX };
  for (uint8_t i = 0; i < 2; i++) {
    int16_t cx = starCX[i];
    tft.drawLine(cx-13, EY, cx+13, EY, COLOR_YELLOW);
    tft.drawLine(cx, EY-13, cx, EY+13, COLOR_YELLOW);
    tft.drawLine(cx-9, EY-9, cx+9, EY+9, COLOR_YELLOW);
    tft.drawLine(cx-9, EY+9, cx+9, EY-9, COLOR_YELLOW);
  }
  drawMouth_Excited();
  drawLabel("EXCITED", COLOR_YELLOW);
}

void showNeutral() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY,  0, C_OUTLINE);
  drawRightBrow(BROW_CY, 0, C_OUTLINE);
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

  drawHead();   // static – full screen face + inset ears + antenna
  drawNose();   // static – filled triangle nose
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
