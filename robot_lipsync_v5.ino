/*
  Robot Face – Serial Lip-Sync + Eye-Sync + Emotion Detection  (v5)
  Display : ILI9225 TFT (176 × 220)
  Library : TFT_22_ILI9225

  HOW IT WORKS
  ─────────────
  1. Type any text in Serial Monitor and press Send (newline-terminated).
  2. The robot detects the EMOTION of the message (happy/sad/angry/surprised/
     excited/scared/disgusted/sleepy/neutral) from keywords.
  3. The face snaps to that emotion.
  4. The mouth LIP-SYNCS through every character:
       vowel  → mouth open  (ellipse, size varies A/E/I/O/U)
       space  → mouth half-open (pause)
       consonant → mouth closed briefly
  5. Eyes BLINK naturally every ~3-4 seconds, and pupils
     dart left/right while "reading" the message.
  6. After talking, the robot returns to IDLE (neutral + slow blink loop).

  SERIAL : 9600 baud, newline terminator
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
#define C_EAR     0x3A77
#define C_EYE_W   COLOR_WHITE
#define C_EYE_P   0x001F
#define C_NOSE    0x7BEF
#define C_LIP     COLOR_RED
#define C_SWEAT   0x07FF
#define C_TEETH   COLOR_WHITE
#define C_INNER   COLOR_BLACK

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
#define BROW_CY 67
#define BROW_PAD 10

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

// ── State ──────────────────────────────────────────────────────────────────
enum Emotion { EMO_NEUTRAL=0, EMO_HAPPY, EMO_SAD, EMO_ANGRY,
               EMO_SURPRISED, EMO_SCARED, EMO_DISGUSTED,
               EMO_SLEEPY, EMO_EXCITED };

Emotion  currentEmo    = EMO_NEUTRAL;
bool     isTalking     = false;
uint32_t lastBlinkTime = 0;
uint32_t blinkInterval = 3500;   // ms between blinks
int16_t  pupilOX       = 0;      // current pupil X offset
int16_t  pupilOY       = 0;      // current pupil Y offset

// ══════════════════════════════════════════════════════════════════════════
//  ELLIPSE ENGINE
// ══════════════════════════════════════════════════════════════════════════
void fillEllipse(int16_t cx, int16_t cy, int16_t rx, int16_t ry, uint16_t col) {
  if (!rx || !ry) return;
  int32_t rx2=(int32_t)rx*rx, ry2=(int32_t)ry*ry;
  int16_t x=0, y=ry;
  int32_t px=0, py=2*rx2*y;
  int32_t p=ry2-rx2*ry+rx2/4;
  while (px<py) {
    tft.drawLine(cx-x,cy+y,cx+x,cy+y,col);
    tft.drawLine(cx-x,cy-y,cx+x,cy-y,col);
    x++; px+=2*ry2;
    if(p<0){p+=ry2+px;}
    else{y--;py-=2*rx2;p+=ry2+px-py;}
  }
  p=ry2*x*x+rx2*(y-1)*(y-1)-rx2*ry2;
  while (y>=0) {
    tft.drawLine(cx-x,cy+y,cx+x,cy+y,col);
    tft.drawLine(cx-x,cy-y,cx+x,cy-y,col);
    y--;py-=2*rx2;
    if(p>0){p+=rx2-py;}
    else{x++;px+=2*ry2;p+=rx2-py+px;}
  }
}

void drawEllipseOutline(int16_t cx, int16_t cy, int16_t rx, int16_t ry, uint16_t col) {
  if (!rx || !ry) return;
  int32_t rx2=(int32_t)rx*rx, ry2=(int32_t)ry*ry;
  int16_t x=0, y=ry;
  int32_t px=0, py=2*rx2*y;
  int32_t p=ry2-rx2*ry+rx2/4;
  while (px<py) {
    tft.drawPixel(cx+x,cy+y,col);tft.drawPixel(cx-x,cy+y,col);
    tft.drawPixel(cx+x,cy-y,col);tft.drawPixel(cx-x,cy-y,col);
    x++;px+=2*ry2;
    if(p<0){p+=ry2+px;}
    else{y--;py-=2*rx2;p+=ry2+px-py;}
  }
  p=ry2*x*x+rx2*(y-1)*(y-1)-rx2*ry2;
  while (y>=0) {
    tft.drawPixel(cx+x,cy+y,col);tft.drawPixel(cx-x,cy+y,col);
    tft.drawPixel(cx+x,cy-y,col);tft.drawPixel(cx-x,cy-y,col);
    y--;py-=2*rx2;
    if(p>0){p+=rx2-py;}
    else{x++;px+=2*ry2;p+=rx2-py+px;}
  }
}

static uint8_t  _arcHalves;
static uint16_t _arcCol;
static int16_t  _arcCX, _arcCY;
void _arcPlot(int16_t dx, int16_t dy) {
  if ((dy>0&&(_arcHalves&0x01))||(dy<0&&(_arcHalves&0x02))||dy==0)
    tft.drawPixel(_arcCX+dx,_arcCY+dy,_arcCol);
}
void drawEllipseArc(int16_t cx, int16_t cy, int16_t rx, int16_t ry,
                    uint8_t halves, uint16_t col) {
  if (!rx||!ry) return;
  _arcCX=cx;_arcCY=cy;_arcHalves=halves;_arcCol=col;
  int32_t rx2=(int32_t)rx*rx, ry2=(int32_t)ry*ry;
  int16_t x=0, y=ry;
  int32_t px=0, py=2*rx2*y;
  int32_t p=ry2-rx2*ry+rx2/4;
  while(px<py){
    _arcPlot(x,y);_arcPlot(-x,y);_arcPlot(x,-y);_arcPlot(-x,-y);
    x++;px+=2*ry2;
    if(p<0){p+=ry2+px;}else{y--;py-=2*rx2;p+=ry2+px-py;}
  }
  p=ry2*x*x+rx2*(y-1)*(y-1)-rx2*ry2;
  while(y>=0){
    _arcPlot(x,y);_arcPlot(-x,y);_arcPlot(x,-y);_arcPlot(-x,-y);
    y--;py-=2*rx2;
    if(p>0){p+=rx2-py;}else{x++;px+=2*ry2;p+=rx2-py+px;}
  }
}
void thickArc(int16_t cx, int16_t cy, int16_t rx, int16_t ry,
              uint8_t halves, uint16_t col) {
  drawEllipseArc(cx,cy,rx,ry,halves,col);
  drawEllipseArc(cx,cy,rx,ry+1,halves,col);
  drawEllipseArc(cx,cy,rx+1,ry,halves,col);
  drawEllipseArc(cx,cy,rx+1,ry+1,halves,col);
}

// ══════════════════════════════════════════════════════════════════════════
//  ERASERS
// ══════════════════════════════════════════════════════════════════════════
void eraseBrow(int16_t x1, int16_t x2, int16_t cy, int16_t slope) {
  int16_t pad=abs(slope)+BROW_PAD;
  tft.fillRectangle(x1-2,cy-pad,x2+2,cy+pad,C_FACE);
}
void eraseEye(int16_t cx, int16_t cy, int16_t erx, int16_t ery) {
  tft.fillRectangle(cx-erx-3,cy-ery-3,cx+erx+3,cy+ery+3,C_FACE);
}
void eraseBlob(int16_t cx, int16_t cy, int16_t r) {
  tft.fillRectangle(cx-r-1,cy-r-1,cx+r+1,cy+r+1,C_FACE);
}
void eraseMouth() {
  tft.fillRectangle(MX-M_RX-2,MY-M_RY-4,MX+M_RX+2,MY+M_RY+4,C_FACE);
}
void eraseLabel() {
  tft.fillRectangle(1,LBL_Y-1,174,LBL_Y+LBL_H+1,C_FACE);
}
void eraseZzz() {
  tft.fillRectangle(130,30,175,75,C_FACE);
}
void clearSweat() {
  eraseBlob(SW_L_X,SW_L_Y,7);
  eraseBlob(SW_R_X,SW_R_Y,7);
  eraseBlob(SC_L_X,SC_L_Y,6);
  eraseBlob(SC_R_X,SC_R_Y,6);
}

// ══════════════════════════════════════════════════════════════════════════
//  STATIC ELEMENTS
// ══════════════════════════════════════════════════════════════════════════
void drawHead() {
  tft.fillRectangle(0,0,175,219,C_FACE);
  tft.drawRectangle(0,0,175,219,C_OUTLINE);
  tft.fillRectangle(EAR_LX1,EAR_Y1,EAR_LX2,EAR_Y2,C_EAR);
  tft.drawRectangle(EAR_LX1,EAR_Y1,EAR_LX2,EAR_Y2,C_OUTLINE);
  int16_t earMidY=(EAR_Y1+EAR_Y2)/2;
  tft.drawLine(EAR_LX1+1,earMidY,EAR_LX2-1,earMidY,C_OUTLINE);
  tft.fillRectangle(EAR_RX1,EAR_Y1,EAR_RX2,EAR_Y2,C_EAR);
  tft.drawRectangle(EAR_RX1,EAR_Y1,EAR_RX2,EAR_Y2,C_OUTLINE);
  tft.drawLine(EAR_RX1+1,earMidY,EAR_RX2-1,earMidY,C_OUTLINE);
  tft.fillRectangle(84,8,92,22,C_OUTLINE);
  fillEllipse(88,5,8,8,COLOR_YELLOW);
  drawEllipseOutline(88,5,8,8,C_OUTLINE);
}

void drawNose() {
  fillEllipse(NTX,((NTY+NBY)/2),6,((NBY-NTY)/2)+2,C_NOSE);
  tft.drawLine(NTX,NTY,NBL,NBY,C_OUTLINE);
  tft.drawLine(NTX,NTY,NBR,NBY,C_OUTLINE);
  tft.drawLine(NBL,NBY,NBR,NBY,C_OUTLINE);
}

// ══════════════════════════════════════════════════════════════════════════
//  BROWS
// ══════════════════════════════════════════════════════════════════════════
void drawLeftBrow(int16_t cy, int16_t slope, uint16_t col) {
  eraseBrow(LBX1,LBX2,cy,slope);
  tft.drawLine(LBX1,cy-slope,LBX2,cy+slope,col);
  tft.drawLine(LBX1,cy-slope+1,LBX2,cy+slope+1,col);
  tft.drawLine(LBX1,cy-slope+2,LBX2,cy+slope+2,col);
}
void drawRightBrow(int16_t cy, int16_t slope, uint16_t col) {
  eraseBrow(RBX1,RBX2,cy,slope);
  tft.drawLine(RBX1,cy+slope,RBX2,cy-slope,col);
  tft.drawLine(RBX1,cy+slope+1,RBX2,cy-slope+1,col);
  tft.drawLine(RBX1,cy+slope+2,RBX2,cy-slope+2,col);
}

// ══════════════════════════════════════════════════════════════════════════
//  EYES  (low-level – draws one eye with pupil at offset pox,poy)
// ══════════════════════════════════════════════════════════════════════════
void drawOneEye(int16_t cx, int16_t cy,
                int16_t erx, int16_t ery,
                int16_t prx, int16_t pry,
                int16_t pox, int16_t poy,
                bool closed, int16_t lidPx, bool angryTint) {
  eraseEye(cx,cy,erx,ery);
  fillEllipse(cx,cy,erx,ery,C_EYE_W);
  drawEllipseOutline(cx,cy,erx,ery,C_OUTLINE);
  if (closed) {
    tft.drawLine(cx-erx,cy,cx+erx,cy,C_OUTLINE);
    tft.drawLine(cx-erx,cy+1,cx+erx,cy+1,C_OUTLINE);
  } else {
    fillEllipse(cx+pox,cy+poy,prx,pry,C_EYE_P);
    drawEllipseOutline(cx+pox,cy+poy,prx,pry,C_OUTLINE);
  }
  if (lidPx>0) {
    tft.fillRectangle(cx-erx-1,cy-ery-1,cx+erx+1,cy-ery+lidPx,C_FACE);
    tft.drawLine(cx-erx-1,cy-ery+lidPx,cx+erx+1,cy-ery+lidPx,C_OUTLINE);
  }
  if (angryTint) {
    tft.drawRectangle(cx-erx-3,cy-ery-3,cx+erx+3,cy+ery+3,COLOR_RED);
    tft.drawRectangle(cx-erx-2,cy-ery-2,cx+erx+2,cy+ery+2,COLOR_RED);
  }
}

// ── Convenience: draw BOTH eyes with current pupil offset ────────────────
void drawEyes(int16_t pox, int16_t poy,
              bool closed=false, int16_t lidPx=0, bool angryTint=false,
              int16_t erx=ERX, int16_t ery=ERY,
              int16_t prx=PRX, int16_t pry=PRY) {
  drawOneEye(LEX,EY,erx,ery,prx,pry,pox,poy,closed,lidPx,angryTint);
  drawOneEye(REX,EY,erx,ery,prx,pry,pox,poy,closed,lidPx,angryTint);
}

// ── BLINK animation ───────────────────────────────────────────────────────
void doBlink(int16_t pox, int16_t poy) {
  // close
  drawEyes(pox,poy,true);
  delay(120);
  // open
  drawEyes(pox,poy,false);
}

// ── PUPIL DART – move pupils to new position smoothly ────────────────────
void movePupils(int16_t newOX, int16_t newOY) {
  pupilOX = newOX;
  pupilOY = newOY;
  drawEyes(pupilOX, pupilOY);
}

// ══════════════════════════════════════════════════════════════════════════
//  MOUTHS
// ══════════════════════════════════════════════════════════════════════════
void drawMouth_Happy() {
  eraseMouth();
  thickArc(MX,MY-10,M_RX,12,0x01,C_LIP);
}
void drawMouth_Sad() {
  eraseMouth();
  thickArc(MX,MY+10,M_RX,12,0x02,C_LIP);
}
void drawMouth_Angry() {
  eraseMouth();
  for(int16_t i=0;i<3;i++) tft.drawLine(MX-28,MY+i,MX+28,MY+i,C_LIP);
}
void drawMouth_Surprised() {
  eraseMouth();
  fillEllipse(MX,MY,16,13,C_BG);
  drawEllipseOutline(MX,MY,16,13,C_LIP);
  drawEllipseOutline(MX,MY,15,12,C_LIP);
}
void drawMouth_Scared() {
  eraseMouth();
  for(int16_t x=-30;x<=30;x+=2) {
    int16_t y=(((x+30)/6)%2==0)?3:-3;
    tft.drawPixel(MX+x,MY+y,C_LIP);
    tft.drawPixel(MX+x,MY+y+1,C_LIP);
    tft.drawPixel(MX+x,MY+y+2,C_LIP);
  }
}
void drawMouth_Disgusted() {
  eraseMouth();
  tft.drawLine(MX-30,MY-8,MX,MY+4,C_LIP);
  tft.drawLine(MX,MY+4,MX+30,MY,C_LIP);
  tft.drawLine(MX-30,MY-7,MX,MY+5,C_LIP);
  tft.drawLine(MX,MY+5,MX+30,MY+1,C_LIP);
}
void drawMouth_Sleepy() {
  eraseMouth();
  thickArc(MX,MY-5,20,8,0x01,C_LIP);
}
void drawMouth_Excited() {
  eraseMouth();
  tft.fillRectangle(MX-M_RX,MY-M_RY+2,MX+M_RX,MY+M_RY-2,C_TEETH);
  tft.drawRectangle(MX-M_RX,MY-M_RY+2,MX+M_RX,MY+M_RY-2,C_LIP);
  tft.drawLine(MX,MY-M_RY+2,MX,MY+M_RY-2,C_LIP);
}
void drawMouth_Neutral() {
  eraseMouth();
  for(int16_t i=0;i<3;i++) tft.drawLine(MX-24,MY+i,MX+24,MY+i,C_LIP);
}

// ── LIP-SYNC MOUTHS (open size 0=closed … 4=wide open) ───────────────────
//   Uses emotion-appropriate color/shape but animated open amount
void drawMouthOpen(uint8_t openSize, Emotion emo) {
  // openSize: 0=closed, 1=slight, 2=mid, 3=open, 4=wide
  static const int16_t ryTable[5] = { 0, 3, 6, 10, 14 };
  static const int16_t rxTable[5] = { 0, 8, 18, 26, 33 };
  int16_t orx = rxTable[openSize];
  int16_t ory = ryTable[openSize];

  eraseMouth();

  if (openSize == 0) {
    // Closed – draw emotion mouth
    switch(emo) {
      case EMO_HAPPY:    drawMouth_Happy();    return;
      case EMO_SAD:      drawMouth_Sad();      return;
      case EMO_ANGRY:    drawMouth_Angry();    return;
      case EMO_DISGUSTED:drawMouth_Disgusted();return;
      case EMO_SLEEPY:   drawMouth_Sleepy();   return;
      case EMO_EXCITED:  drawMouth_Excited();  return;
      default:           drawMouth_Neutral();  return;
    }
  }

  // Open mouth: filled black oval + lip outline + teeth line
  fillEllipse(MX,MY,orx,ory,C_INNER);
  drawEllipseOutline(MX,MY,orx,ory,C_LIP);
  if (ory >= 6) {
    // Show teeth line
    tft.drawLine(MX-orx+4, MY-ory/3, MX+orx-4, MY-ory/3, C_TEETH);
  }
  // Emotion tint on lips
  if (emo==EMO_HAPPY || emo==EMO_EXCITED) {
    drawEllipseArc(MX,MY-ory/2,orx,ory/3+1,0x01,C_LIP);
  }
}

// ══════════════════════════════════════════════════════════════════════════
//  LABEL
// ══════════════════════════════════════════════════════════════════════════
void drawLabel(const char* text, uint16_t col) {
  eraseLabel();
  tft.setFont(Terminal6x8);
  int16_t tx=88-((int16_t)strlen(text)*6)/2;
  tft.drawText(tx,LBL_Y,text,col);
}

void drawZzz() {
  tft.setFont(Terminal6x8);
  tft.drawText(138,60,"z",COLOR_CYAN);
  tft.drawText(150,48,"Z",COLOR_CYAN);
  tft.drawText(163,35,"Z",COLOR_CYAN);
}

// ══════════════════════════════════════════════════════════════════════════
//  EXPRESSION SETTERS  (brows + eyes + base mouth + label)
// ══════════════════════════════════════════════════════════════════════════
void applyEmotion(Emotion emo) {
  clearSweat(); eraseZzz();
  switch(emo) {
    case EMO_HAPPY:
      drawLeftBrow(BROW_CY-4,-5,C_OUTLINE);
      drawRightBrow(BROW_CY-4,-5,C_OUTLINE);
      drawEyes(0,3);
      drawMouth_Happy();
      drawLabel("HAPPY",COLOR_YELLOW);
      break;
    case EMO_SAD:
      drawLeftBrow(BROW_CY+3,6,C_OUTLINE);
      drawRightBrow(BROW_CY+3,6,C_OUTLINE);
      drawEyes(0,3);
      drawMouth_Sad();
      fillEllipse(SW_L_X,SW_L_Y,4,5,C_SWEAT);
      fillEllipse(SW_R_X,SW_R_Y,4,5,C_SWEAT);
      drawLabel("SAD",0x07FF);
      break;
    case EMO_ANGRY:
      drawLeftBrow(BROW_CY+2,7,COLOR_RED);
      drawRightBrow(BROW_CY+2,7,COLOR_RED);
      drawEyes(0,4,false,0,true);
      drawMouth_Angry();
      drawLabel("ANGRY",COLOR_RED);
      break;
    case EMO_SURPRISED:
      drawLeftBrow(BROW_CY-8,-7,C_OUTLINE);
      drawRightBrow(BROW_CY-8,-7,C_OUTLINE);
      drawEyes(0,0,false,0,false,ERX+3,ERY+4,PRX+1,PRY+1);
      drawMouth_Surprised();
      drawLabel("SURPRISED",COLOR_YELLOW);
      break;
    case EMO_SCARED:
      drawLeftBrow(BROW_CY-6,-6,C_OUTLINE);
      drawRightBrow(BROW_CY-6,-6,C_OUTLINE);
      drawEyes(4,-4,false,0,false,ERX+3,ERY+4,PRX-1,PRY-1);
      drawMouth_Scared();
      fillEllipse(SC_L_X,SC_L_Y,4,5,C_SWEAT);
      fillEllipse(SC_R_X,SC_R_Y,4,5,C_SWEAT);
      drawLabel("SCARED",0x07FF);
      break;
    case EMO_DISGUSTED:
      drawLeftBrow(BROW_CY+3,4,C_OUTLINE);
      drawRightBrow(BROW_CY-1,-3,C_OUTLINE);
      drawOneEye(LEX,EY,ERX,ERY,PRX,PRY,-3,5,false,ERY,false);
      drawOneEye(REX,EY,ERX,ERY,PRX,PRY,0,2,false,0,false);
      drawMouth_Disgusted();
      drawLabel("DISGUSTED",COLOR_GREEN);
      break;
    case EMO_SLEEPY:
      drawLeftBrow(BROW_CY+2,0,C_OUTLINE);
      drawRightBrow(BROW_CY+2,0,C_OUTLINE);
      drawEyes(0,5,false,ERY);
      drawMouth_Sleepy();
      drawZzz();
      drawLabel("SLEEPY",0x7BEF);
      break;
    case EMO_EXCITED:
      drawLeftBrow(BROW_CY-6,-8,COLOR_YELLOW);
      drawRightBrow(BROW_CY-6,-8,COLOR_YELLOW);
      drawEyes(0,0,false,0,false,ERX,ERY,PRX+3,PRY+3);
      {
        const int16_t sc[2]={LEX,REX};
        for(uint8_t i=0;i<2;i++){
          tft.drawLine(sc[i]-13,EY,sc[i]+13,EY,COLOR_YELLOW);
          tft.drawLine(sc[i],EY-13,sc[i],EY+13,COLOR_YELLOW);
          tft.drawLine(sc[i]-9,EY-9,sc[i]+9,EY+9,COLOR_YELLOW);
          tft.drawLine(sc[i]-9,EY+9,sc[i]+9,EY-9,COLOR_YELLOW);
        }
      }
      drawMouth_Excited();
      drawLabel("EXCITED",COLOR_YELLOW);
      break;
    default: // NEUTRAL
      drawLeftBrow(BROW_CY,0,C_OUTLINE);
      drawRightBrow(BROW_CY,0,C_OUTLINE);
      drawEyes(0,0);
      drawMouth_Neutral();
      drawLabel("LISTENING",COLOR_WHITE);
      break;
  }
  pupilOX=0; pupilOY=0;
}

// ══════════════════════════════════════════════════════════════════════════
//  EMOTION DETECTOR  (keyword scan, case-insensitive)
// ══════════════════════════════════════════════════════════════════════════
bool strContains(const char* haystack, const char* needle) {
  uint8_t hl=strlen(haystack), nl=strlen(needle);
  if(nl>hl) return false;
  for(uint8_t i=0;i<=hl-nl;i++){
    bool match=true;
    for(uint8_t j=0;j<nl;j++){
      if(tolower(haystack[i+j])!=tolower(needle[j])){match=false;break;}
    }
    if(match) return true;
  }
  return false;
}

Emotion detectEmotion(const char* msg) {
  // Happy keywords
  if(strContains(msg,"hi")||strContains(msg,"hello")||
     strContains(msg,"haha")||strContains(msg,"lol")||
     strContains(msg,"happy")||strContains(msg,"good")||
     strContains(msg,"great")||strContains(msg,"awesome")||
     strContains(msg,"love")||strContains(msg,"joy")||
     strContains(msg,"yay")||strContains(msg,":)")||
     strContains(msg,"wonderful")||strContains(msg,"nice"))
    return EMO_HAPPY;

  // Excited keywords
  if(strContains(msg,"wow")||strContains(msg,"!")||
     strContains(msg,"omg")||strContains(msg,"amazing")||
     strContains(msg,"excited")||strContains(msg,"fantastic")||
     strContains(msg,"incredible")||strContains(msg,"woah"))
    return EMO_EXCITED;

  // Surprised
  if(strContains(msg,"really")||strContains(msg,"what")||
     strContains(msg,"seriously")||strContains(msg,"no way")||
     strContains(msg,"surprised")||strContains(msg,"whoa"))
    return EMO_SURPRISED;

  // Angry keywords
  if(strContains(msg,"angry")||strContains(msg,"mad")||
     strContains(msg,"hate")||strContains(msg,"stupid")||
     strContains(msg,"idiot")||strContains(msg,"shut")||
     strContains(msg,"damn")||strContains(msg,"rage"))
    return EMO_ANGRY;

  // Sad keywords
  if(strContains(msg,"sad")||strContains(msg,"cry")||
     strContains(msg,"miss")||strContains(msg,"sorry")||
     strContains(msg,"lonely")||strContains(msg,"depress")||
     strContains(msg,"unhappy")||strContains(msg,":(")||
     strContains(msg,"bad")||strContains(msg,"hurt"))
    return EMO_SAD;

  // Scared
  if(strContains(msg,"scared")||strContains(msg,"fear")||
     strContains(msg,"afraid")||strContains(msg,"dark")||
     strContains(msg,"monster")||strContains(msg,"help"))
    return EMO_SCARED;

  // Disgusted
  if(strContains(msg,"ew")||strContains(msg,"gross")||
     strContains(msg,"yuck")||strContains(msg,"disgust")||
     strContains(msg,"awful")||strContains(msg,"sick"))
    return EMO_DISGUSTED;

  // Sleepy
  if(strContains(msg,"sleep")||strContains(msg,"tired")||
     strContains(msg,"bore")||strContains(msg,"yawn")||
     strContains(msg,"night")||strContains(msg,"bed")||
     strContains(msg,"zzz"))
    return EMO_SLEEPY;

  return EMO_NEUTRAL;
}

// ══════════════════════════════════════════════════════════════════════════
//  LIP-SYNC ANIMATION
//  Maps each character to an open-mouth size, animates it
// ══════════════════════════════════════════════════════════════════════════
uint8_t charToOpenSize(char c) {
  c = tolower(c);
  switch(c) {
    case 'a': case 'o': return 4;          // wide open
    case 'e': case 'u': return 3;          // open
    case 'i':           return 2;          // medium
    case 'm': case 'b': case 'p': return 0; // bilabial – fully closed
    case ' ': case ',': case '.': return 1; // pause
    default:            return 2;          // default consonant
  }
}

// Eye movement table – pupils dart across while talking
static const int8_t eyeSeq[][2] = {
  {0,0},{4,0},{-4,0},{0,2},{3,-2},{-3,0},{0,0},{2,2},{-2,-1}
};
uint8_t eyeSeqIdx = 0;

void lipSync(const char* msg, Emotion emo) {
  uint8_t len = strlen(msg);
  uint8_t prevOpen = 0;
  uint8_t eyeStep = 0;

  for (uint8_t i = 0; i < len; i++) {
    char c = msg[i];
    uint8_t openSz = charToOpenSize(c);

    // Animate mouth from previous to target in 1 step
    if (openSz != prevOpen) {
      drawMouthOpen(openSz, emo);
      prevOpen = openSz;
    }

    // Every 4 characters, shift pupils (eye tracking reading effect)
    if ((i % 4) == 0) {
      uint8_t idx = eyeStep % 9;
      int16_t pox = eyeSeq[idx][0];
      int16_t poy = eyeSeq[idx][1];
      // Only redraw pupils if not disgusted/scared (those have fixed offsets)
      if (emo != EMO_DISGUSTED && emo != EMO_SCARED) {
        drawEyes(pox, poy, false, 0,
                 emo==EMO_ANGRY,
                 (emo==EMO_SURPRISED||emo==EMO_SCARED)?ERX+3:ERX,
                 (emo==EMO_SURPRISED||emo==EMO_SCARED)?ERY+4:ERY);
      }
      eyeStep++;
    }

    // Per-char timing: vowels longer, consonants shorter, spaces pause
    uint16_t charDelay;
    if (c == ' ' || c == ',' || c == '.') charDelay = 150;
    else if (openSz >= 3)  charDelay = 110;
    else if (openSz == 0)  charDelay = 70;
    else                   charDelay = 90;
    delay(charDelay);

    // Natural blink check mid-sentence
    if (millis() - lastBlinkTime > blinkInterval) {
      doBlink(pupilOX, pupilOY);
      lastBlinkTime = millis();
      blinkInterval = 3000 + random(1500);
    }
  }

  // Close mouth at end
  drawMouthOpen(0, emo);
}

// ══════════════════════════════════════════════════════════════════════════
//  SETUP & LOOP
// ══════════════════════════════════════════════════════════════════════════
#define MAX_MSG 80
char msgBuf[MAX_MSG+1];
uint8_t msgLen = 0;

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

  Serial.begin(9600);

  drawHead();
  drawNose();
  applyEmotion(EMO_NEUTRAL);

  lastBlinkTime = millis();
  blinkInterval = 3000;

  Serial.println("Robot ready! Type a message and press Enter.");
}

void loop() {
  // ── Read Serial ───────────────────────────────────────────────────────
  while (Serial.available()) {
    char ch = (char)Serial.read();
    if (ch == '\n' || ch == '\r') {
      if (msgLen > 0) {
        msgBuf[msgLen] = '\0';

        // 1. Detect emotion
        Emotion emo = detectEmotion(msgBuf);
        currentEmo = emo;

        // 2. Apply face expression instantly
        applyEmotion(emo);
        delay(300);   // brief pause so face "settles"

        // 3. Blink once (robot "wakes up" to speak)
        doBlink(0,0);
        delay(100);

        // 4. Lip-sync through the message
        isTalking = true;
        lipSync(msgBuf, emo);
        isTalking = false;

        // 5. Hold expression for 1.5s then relax to neutral
        delay(1500);
        applyEmotion(EMO_NEUTRAL);
        drawLabel("LISTENING", COLOR_WHITE);

        msgLen = 0;
      }
    } else {
      if (msgLen < MAX_MSG) {
        msgBuf[msgLen++] = ch;
      }
    }
  }

  // ── IDLE behaviours (when not talking) ───────────────────────────────
  if (!isTalking) {
    uint32_t now = millis();

    // Auto blink
    if (now - lastBlinkTime > blinkInterval) {
      doBlink(pupilOX, pupilOY);
      lastBlinkTime = now;
      blinkInterval = 2800 + random(2000);

      // Occasional idle pupil drift
      int16_t dx = (int8_t)(random(7)) - 3;
      int16_t dy = (int8_t)(random(5)) - 2;
      movePupils(dx, dy);
    }
  }
}
