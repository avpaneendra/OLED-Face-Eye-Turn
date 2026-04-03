/*
  Robot Face - 9 Emotional Expressions
  Display : ILI9225 TFT (176 × 220)
  Library : Arduino_GFX by Moon On Our Nation
  Board   : Arduino UNO R4 WiFi / Minima
*/

#include <Arduino_GFX_Library.h>

// ── Pins ───────────────────────────────────────────────────────────────────
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8
#define TFT_BL    7
/*
 * CONNECTIONS
 * 
 #define TFT_RST 8
#define TFT_RS  9
#define TFT_CS  10  // SS
#define TFT_SDI 11  // MOSI
#define TFT_CLK 13  // SCK
*/

  
// ── GFX objects ────────────────────────────────────────────────────────────
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
Arduino_GFX     *tft = new Arduino_ILI9225(bus, TFT_RST);

// ── Colors (RGB565 hex — no reliance on undefined color names) ─────────────
#define C_BLACK   0x0000
#define C_WHITE   0xFFFF
#define C_RED     0xF800
#define C_GREEN   0x07E0
#define C_BLUE    0x001F
#define C_YELLOW  0xFFE0
#define C_CYAN    0x07FF

#define C_BG      C_BLACK
#define C_FACE    0x4C99
#define C_OUTLINE C_WHITE
#define C_EAR     0x3A77
#define C_EYE_W   C_WHITE
#define C_EYE_P   C_BLUE
#define C_NOSE    0x7BEF
#define C_LIP     C_RED
#define C_SWEAT   C_CYAN

// ── Fixed geometry ─────────────────────────────────────────────────────────
#define LEX   55
#define REX  121
#define EY    90
#define ERX   19
#define ERY   15
#define PRX    8
#define PRY    7

#define LBX1  36
#define LBX2  74
#define RBX1 102
#define RBX2 140
#define BROW_CY   67
#define BROW_PAD  10

#define NTX   88
#define NTY  125
#define NBY  148
#define NBL   80
#define NBR   96

#define MX    88
#define MY   178
#define M_RX  34
#define M_RY  16

#define LBL_Y 207
#define LBL_H  10

#define EAR_LX1   4
#define EAR_LX2  22
#define EAR_RX1 153
#define EAR_RX2 171
#define EAR_Y1   80
#define EAR_Y2  150

#define SW_L_X  (LEX - 4)
#define SW_L_Y  (EY + 22)
#define SW_R_X  (REX + 4)
#define SW_R_Y  (EY + 22)
#define SC_L_X  25
#define SC_L_Y  85
#define SC_R_X 151
#define SC_R_Y  85

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
    tft->drawFastHLine(cx-x, cy+y, 2*x+1, col);
    tft->drawFastHLine(cx-x, cy-y, 2*x+1, col);
    x++; px += 2*ry2;
    if (p < 0) { p += ry2+px; }
    else { y--; py -= 2*rx2; p += ry2+px-py; }
  }
  p = ry2*x*x + rx2*(y-1)*(y-1) - rx2*ry2;
  while (y >= 0) {
    tft->drawFastHLine(cx-x, cy+y, 2*x+1, col);
    tft->drawFastHLine(cx-x, cy-y, 2*x+1, col);
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
    tft->drawPixel(cx+x,cy+y,col); tft->drawPixel(cx-x,cy+y,col);
    tft->drawPixel(cx+x,cy-y,col); tft->drawPixel(cx-x,cy-y,col);
    x++; px += 2*ry2;
    if (p < 0) { p += ry2+px; }
    else { y--; py -= 2*rx2; p += ry2+px-py; }
  }
  p = ry2*x*x + rx2*(y-1)*(y-1) - rx2*ry2;
  while (y >= 0) {
    tft->drawPixel(cx+x,cy+y,col); tft->drawPixel(cx-x,cy+y,col);
    tft->drawPixel(cx+x,cy-y,col); tft->drawPixel(cx-x,cy-y,col);
    y--; py -= 2*rx2;
    if (p > 0) { p += rx2-py; }
    else { x++; px += 2*ry2; p += rx2-py+px; }
  }
}

static uint8_t  _arcHalves;
static uint16_t _arcCol;
static int16_t  _arcCX, _arcCY;

void _arcPlot(int16_t dx, int16_t dy) {
  if ((dy > 0 && (_arcHalves & 0x01)) ||
      (dy < 0 && (_arcHalves & 0x02)) ||
       dy == 0)
    tft->drawPixel(_arcCX + dx, _arcCY + dy, _arcCol);
}

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
//  ERASERS
// ══════════════════════════════════════════════════════════════════════════
void eraseBrow(int16_t x1, int16_t x2, int16_t cy, int16_t slope) {
  int16_t pad = abs(slope) + BROW_PAD;
  tft->fillRect(x1-2, cy-pad, (x2+2)-(x1-2), pad*2, C_FACE);
}
void eraseEye(int16_t cx, int16_t cy, int16_t erx, int16_t ery) {
  tft->fillRect(cx-erx-2, cy-ery-2, (erx+2)*2, (ery+2)*2, C_FACE);
}
void eraseBlob(int16_t cx, int16_t cy, int16_t r) {
  tft->fillRect(cx-r-1, cy-r-1, (r+1)*2, (r+1)*2, C_FACE);
}
void eraseMouth() {
  tft->fillRect(MX-M_RX-2, MY-M_RY-2, (M_RX+2)*2, (M_RY+2)*2, C_FACE);
}
void eraseLabel() {
  tft->fillRect(1, LBL_Y-1, 173, LBL_H+2, C_FACE);
}
void eraseZzz() {
  tft->fillRect(130, 30, 45, 45, C_FACE);
}

// ══════════════════════════════════════════════════════════════════════════
//  STATIC ELEMENTS
// ══════════════════════════════════════════════════════════════════════════
void drawHead() {
  tft->fillScreen(C_FACE);
  tft->drawRect(0, 0, 176, 220, C_OUTLINE);

  tft->fillRect(EAR_LX1, EAR_Y1, EAR_LX2-EAR_LX1, EAR_Y2-EAR_Y1, C_EAR);
  tft->drawRect(EAR_LX1, EAR_Y1, EAR_LX2-EAR_LX1, EAR_Y2-EAR_Y1, C_OUTLINE);
  int16_t earMidY = (EAR_Y1 + EAR_Y2) / 2;
  tft->drawFastHLine(EAR_LX1+1, earMidY, EAR_LX2-EAR_LX1-2, C_OUTLINE);

  tft->fillRect(EAR_RX1, EAR_Y1, EAR_RX2-EAR_RX1, EAR_Y2-EAR_Y1, C_EAR);
  tft->drawRect(EAR_RX1, EAR_Y1, EAR_RX2-EAR_RX1, EAR_Y2-EAR_Y1, C_OUTLINE);
  tft->drawFastHLine(EAR_RX1+1, earMidY, EAR_RX2-EAR_RX1-2, C_OUTLINE);

  tft->fillRect(84, 8, 8, 14, C_OUTLINE);
  fillEllipse(88, 5, 8, 8, C_YELLOW);
  drawEllipseOutline(88, 5, 8, 8, C_OUTLINE);
}

void drawNose() {
  for (int16_t y = NTY; y <= NBY; y++) {
    float t = (float)(y - NTY) / (NBY - NTY);
    int16_t xl = NTX - (int16_t)(t * (NTX - NBL));
    int16_t xr = NTX + (int16_t)(t * (NBR - NTX));
    tft->drawFastHLine(xl, y, xr-xl, C_NOSE);
  }
  tft->drawLine(NTX, NTY, NBL, NBY, C_OUTLINE);
  tft->drawLine(NTX, NTY, NBR, NBY, C_OUTLINE);
  tft->drawFastHLine(NBL, NBY, NBR-NBL, C_OUTLINE);
}

// ══════════════════════════════════════════════════════════════════════════
//  ORGAN DRAW FUNCTIONS
// ══════════════════════════════════════════════════════════════════════════
void drawLeftBrow(int16_t cy, int16_t slope, uint16_t col) {
  eraseBrow(LBX1, LBX2, cy, slope);
  tft->drawLine(LBX1, cy-slope,   LBX2, cy+slope,   col);
  tft->drawLine(LBX1, cy-slope+1, LBX2, cy+slope+1, col);
  tft->drawLine(LBX1, cy-slope+2, LBX2, cy+slope+2, col);
}
void drawRightBrow(int16_t cy, int16_t slope, uint16_t col) {
  eraseBrow(RBX1, RBX2, cy, slope);
  tft->drawLine(RBX1, cy+slope,   RBX2, cy-slope,   col);
  tft->drawLine(RBX1, cy+slope+1, RBX2, cy-slope+1, col);
  tft->drawLine(RBX1, cy+slope+2, RBX2, cy-slope+2, col);
}

void drawOneEye(int16_t cx, int16_t cy,
                int16_t erx, int16_t ery,
                int16_t prx, int16_t pry,
                int16_t pox, int16_t poy,
                bool closed, int16_t lidPx, bool angryTint) {
  eraseEye(cx, cy, erx, ery);
  fillEllipse(cx, cy, erx, ery, C_EYE_W);
  drawEllipseOutline(cx, cy, erx, ery, C_OUTLINE);
  if (closed) {
    tft->drawFastHLine(cx-erx, cy,   erx*2, C_OUTLINE);
    tft->drawFastHLine(cx-erx, cy+1, erx*2, C_OUTLINE);
  } else {
    fillEllipse(cx+pox, cy+poy, prx, pry, C_EYE_P);
    drawEllipseOutline(cx+pox, cy+poy, prx, pry, C_OUTLINE);
  }
  if (lidPx > 0) {
    tft->fillRect(cx-erx-1, cy-ery-1, (erx+1)*2, lidPx+1, C_FACE);
    tft->drawFastHLine(cx-erx-1, cy-ery+lidPx, (erx+1)*2, C_OUTLINE);
  }
  if (angryTint) {
    tft->drawRect(cx-erx-3, cy-ery-3, (erx+3)*2, (ery+3)*2, C_RED);
    tft->drawRect(cx-erx-2, cy-ery-2, (erx+2)*2, (ery+2)*2, C_RED);
  }
}

void drawMouth_Happy()  { eraseMouth(); thickArc(MX, MY-10, M_RX, 12, 0x01, C_LIP); }
void drawMouth_Sad()    { eraseMouth(); thickArc(MX, MY+10, M_RX, 12, 0x02, C_LIP); }
void drawMouth_Sleepy() { eraseMouth(); thickArc(MX, MY-5,  20,    8, 0x01, C_LIP); }

void drawMouth_Angry() {
  eraseMouth();
  for (int16_t i = 0; i < 3; i++)
    tft->drawFastHLine(MX-28, MY+i, 56, C_LIP);
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
    tft->drawPixel(MX+x, MY+y,   C_LIP);
    tft->drawPixel(MX+x, MY+y+1, C_LIP);
    tft->drawPixel(MX+x, MY+y+2, C_LIP);
  }
}
void drawMouth_Disgusted() {
  eraseMouth();
  tft->drawLine(MX-30, MY-8, MX,    MY+4, C_LIP);
  tft->drawLine(MX,    MY+4, MX+30, MY,   C_LIP);
  tft->drawLine(MX-30, MY-7, MX,    MY+5, C_LIP);
  tft->drawLine(MX,    MY+5, MX+30, MY+1, C_LIP);
}
void drawMouth_Excited() {
  eraseMouth();
  tft->fillRect(MX-M_RX, MY-M_RY+2, M_RX*2, (M_RY-2)*2, C_WHITE);
  tft->drawRect(MX-M_RX, MY-M_RY+2, M_RX*2, (M_RY-2)*2, C_LIP);
  tft->drawFastVLine(MX, MY-M_RY+2, (M_RY-2)*2, C_LIP);
}
void drawMouth_Neutral() {
  eraseMouth();
  for (int16_t i = 0; i < 3; i++)
    tft->drawFastHLine(MX-24, MY+i, 48, C_LIP);
}

void drawLabel(const char* text, uint16_t col) {
  eraseLabel();
  tft->setTextColor(col);
  tft->setTextSize(1);
  int16_t tx = 88 - ((int16_t)strlen(text) * 6) / 2;
  tft->setCursor(tx, LBL_Y);
  tft->print(text);
}

void drawZzz() {
  tft->setTextColor(C_CYAN);
  tft->setTextSize(1);
  tft->setCursor(138, 60); tft->print("z");
  tft->setCursor(150, 48); tft->print("Z");
  tft->setCursor(163, 35); tft->print("Z");
}

// ══════════════════════════════════════════════════════════════════════════
//  EXPRESSIONS
// ══════════════════════════════════════════════════════════════════════════
void clearSweat() {
  eraseBlob(SW_L_X, SW_L_Y, 7); eraseBlob(SW_R_X, SW_R_Y, 7);
  eraseBlob(SC_L_X, SC_L_Y, 6); eraseBlob(SC_R_X, SC_R_Y, 6);
}

void showHappy() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY-4, -5, C_OUTLINE); drawRightBrow(BROW_CY-4, -5, C_OUTLINE);
  drawOneEye(LEX,EY,ERX,ERY,PRX,PRY, 0,3,false,0,false);
  drawOneEye(REX,EY,ERX,ERY,PRX,PRY, 0,3,false,0,false);
  drawMouth_Happy(); drawLabel("HAPPY", C_YELLOW);
}
void showSad() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY+3, 6, C_OUTLINE); drawRightBrow(BROW_CY+3, 6, C_OUTLINE);
  drawOneEye(LEX,EY,ERX,ERY,PRX,PRY, 0,3,false,0,false);
  drawOneEye(REX,EY,ERX,ERY,PRX,PRY, 0,3,false,0,false);
  drawMouth_Sad();
  fillEllipse(SW_L_X,SW_L_Y,4,5,C_SWEAT);
  fillEllipse(SW_R_X,SW_R_Y,4,5,C_SWEAT);
  drawLabel("SAD", C_CYAN);
}
void showAngry() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY+2, 7, C_RED); drawRightBrow(BROW_CY+2, 7, C_RED);
  drawOneEye(LEX,EY,ERX,ERY,PRX,PRY, 0,4,false,0,true);
  drawOneEye(REX,EY,ERX,ERY,PRX,PRY, 0,4,false,0,true);
  drawMouth_Angry(); drawLabel("ANGRY", C_RED);
}
void showSurprised() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY-8,-7,C_OUTLINE); drawRightBrow(BROW_CY-8,-7,C_OUTLINE);
  drawOneEye(LEX,EY,ERX+3,ERY+4,PRX+1,PRY+1,0,0,false,0,false);
  drawOneEye(REX,EY,ERX+3,ERY+4,PRX+1,PRY+1,0,0,false,0,false);
  drawMouth_Surprised(); drawLabel("SURPRISED", C_YELLOW);
}
void showScared() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY-6,-6,C_OUTLINE); drawRightBrow(BROW_CY-6,-6,C_OUTLINE);
  drawOneEye(LEX,EY,ERX+3,ERY+4,PRX-1,PRY-1, 4,-4,false,0,false);
  drawOneEye(REX,EY,ERX+3,ERY+4,PRX-1,PRY-1, 4,-4,false,0,false);
  drawMouth_Scared();
  fillEllipse(SC_L_X,SC_L_Y,4,5,C_SWEAT);
  fillEllipse(SC_R_X,SC_R_Y,4,5,C_SWEAT);
  drawLabel("SCARED", C_CYAN);
}
void showDisgusted() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY+3, 4,C_OUTLINE); drawRightBrow(BROW_CY-1,-3,C_OUTLINE);
  drawOneEye(LEX,EY,ERX,ERY,PRX,PRY,-3, 5,false,ERY,false);
  drawOneEye(REX,EY,ERX,ERY,PRX,PRY, 0, 2,false,  0,false);
  drawMouth_Disgusted(); drawLabel("DISGUSTED", C_GREEN);
}
void showSleepy() {
  clearSweat();
  drawLeftBrow(BROW_CY+2,0,C_OUTLINE); drawRightBrow(BROW_CY+2,0,C_OUTLINE);
  drawOneEye(LEX,EY,ERX,ERY,PRX,PRY,0,5,false,ERY,false);
  drawOneEye(REX,EY,ERX,ERY,PRX,PRY,0,5,false,ERY,false);
  drawMouth_Sleepy(); drawZzz(); drawLabel("SLEEPY", 0x7BEF);
}
void showExcited() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY-6,-8,C_YELLOW); drawRightBrow(BROW_CY-6,-8,C_YELLOW);
  drawOneEye(LEX,EY,ERX,ERY,PRX+3,PRY+3,0,0,false,0,false);
  drawOneEye(REX,EY,ERX,ERY,PRX+3,PRY+3,0,0,false,0,false);
  const int16_t starCX[2] = {LEX, REX};
  for (uint8_t i = 0; i < 2; i++) {
    int16_t cx = starCX[i];
    tft->drawLine(cx-13,EY,cx+13,EY,C_YELLOW);
    tft->drawLine(cx,EY-13,cx,EY+13,C_YELLOW);
    tft->drawLine(cx-9,EY-9,cx+9,EY+9,C_YELLOW);
    tft->drawLine(cx-9,EY+9,cx+9,EY-9,C_YELLOW);
  }
  drawMouth_Excited(); drawLabel("EXCITED", C_YELLOW);
}
void showNeutral() {
  clearSweat(); eraseZzz();
  drawLeftBrow(BROW_CY,0,C_OUTLINE); drawRightBrow(BROW_CY,0,C_OUTLINE);
  drawOneEye(LEX,EY,ERX,ERY,PRX,PRY,0,0,false,0,false);
  drawOneEye(REX,EY,ERX,ERY,PRX,PRY,0,0,false,0,false);
  drawMouth_Neutral(); drawLabel("NEUTRAL", C_WHITE);
}

// ══════════════════════════════════════════════════════════════════════════
//  SETUP & LOOP
// ══════════════════════════════════════════════════════════════════════════
void setup() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft->begin();
  tft->setRotation(0);
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
