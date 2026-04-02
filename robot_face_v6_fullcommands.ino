/*
  Robot Face v6 – FULL COMMAND SET
  Display : ILI9225 TFT (176 × 220)   Library : TFT_22_ILI9225
  Serial  : 9600 baud, Newline

  ╔══════════════════════════════════════╗
  ║  COMMAND LIST (type in Serial Mon.)  ║
  ╠══════════════════════════════════════╣
  ║  show your tongue                    ║
  ║  show your teeth                     ║
  ║  show your eyes                      ║
  ║  show your nose                      ║
  ║  show your lips                      ║
  ║  kiss                                ║
  ║  see left / see right                ║
  ║  see up   / see down                 ║
  ║  see back / see front                ║
  ║  are you hungry                      ║
  ║  say rhymes  (Twinkle scroll)        ║
  ║  3+4 / 10-2 / 5*6 / 20/4  (math)    ║
  ║  hi / hello / sad / angry … (emotes) ║
  ╚══════════════════════════════════════╝
*/

#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ── Pins ───────────────────────────────────────────────────────────────────
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
#define C_TONGUE  0xF8A0
#define C_KISS    0xFF00
#define C_HEART   COLOR_RED

// ── Geometry ───────────────────────────────────────────────────────────────
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

#define SW_L_X (LEX-4)
#define SW_L_Y (EY+22)
#define SW_R_X (REX+4)
#define SW_R_Y (EY+22)
#define SC_L_X 25
#define SC_L_Y 85
#define SC_R_X 151
#define SC_R_Y 85

// ── State ──────────────────────────────────────────────────────────────────
enum Emotion { EMO_NEUTRAL=0,EMO_HAPPY,EMO_SAD,EMO_ANGRY,
               EMO_SURPRISED,EMO_SCARED,EMO_DISGUSTED,
               EMO_SLEEPY,EMO_EXCITED };

Emotion  currentEmo = EMO_NEUTRAL;
bool     isTalking  = false;
uint32_t lastBlink  = 0;
uint32_t blinkInt   = 3500;
int16_t  pupilOX    = 0;
int16_t  pupilOY    = 0;

// ══════════════════════════════════════════════════════════════════════════
//  ELLIPSE ENGINE
// ══════════════════════════════════════════════════════════════════════════
void fillEllipse(int16_t cx,int16_t cy,int16_t rx,int16_t ry,uint16_t col){
  if(!rx||!ry)return;
  int32_t rx2=(int32_t)rx*rx,ry2=(int32_t)ry*ry;
  int16_t x=0,y=ry;
  int32_t px=0,py=2*rx2*y,p=ry2-rx2*ry+rx2/4;
  while(px<py){
    tft.drawLine(cx-x,cy+y,cx+x,cy+y,col);
    tft.drawLine(cx-x,cy-y,cx+x,cy-y,col);
    x++;px+=2*ry2;
    if(p<0)p+=ry2+px; else{y--;py-=2*rx2;p+=ry2+px-py;}
  }
  p=ry2*x*x+rx2*(y-1)*(y-1)-rx2*ry2;
  while(y>=0){
    tft.drawLine(cx-x,cy+y,cx+x,cy+y,col);
    tft.drawLine(cx-x,cy-y,cx+x,cy-y,col);
    y--;py-=2*rx2;
    if(p>0)p+=rx2-py; else{x++;px+=2*ry2;p+=rx2-py+px;}
  }
}
void drawEllipseOutline(int16_t cx,int16_t cy,int16_t rx,int16_t ry,uint16_t col){
  if(!rx||!ry)return;
  int32_t rx2=(int32_t)rx*rx,ry2=(int32_t)ry*ry;
  int16_t x=0,y=ry;
  int32_t px=0,py=2*rx2*y,p=ry2-rx2*ry+rx2/4;
  while(px<py){
    tft.drawPixel(cx+x,cy+y,col);tft.drawPixel(cx-x,cy+y,col);
    tft.drawPixel(cx+x,cy-y,col);tft.drawPixel(cx-x,cy-y,col);
    x++;px+=2*ry2;
    if(p<0)p+=ry2+px; else{y--;py-=2*rx2;p+=ry2+px-py;}
  }
  p=ry2*x*x+rx2*(y-1)*(y-1)-rx2*ry2;
  while(y>=0){
    tft.drawPixel(cx+x,cy+y,col);tft.drawPixel(cx-x,cy+y,col);
    tft.drawPixel(cx+x,cy-y,col);tft.drawPixel(cx-x,cy-y,col);
    y--;py-=2*rx2;
    if(p>0)p+=rx2-py; else{x++;px+=2*ry2;p+=rx2-py+px;}
  }
}
static uint8_t _arcH; static uint16_t _arcC; static int16_t _arcX,_arcY;
void _arcPlot(int16_t dx,int16_t dy){
  if((dy>0&&(_arcH&1))||(dy<0&&(_arcH&2))||dy==0) tft.drawPixel(_arcX+dx,_arcY+dy,_arcC);
}
void drawEllipseArc(int16_t cx,int16_t cy,int16_t rx,int16_t ry,uint8_t h,uint16_t col){
  if(!rx||!ry)return;
  _arcX=cx;_arcY=cy;_arcH=h;_arcC=col;
  int32_t rx2=(int32_t)rx*rx,ry2=(int32_t)ry*ry;
  int16_t x=0,y=ry;
  int32_t px=0,py=2*rx2*y,p=ry2-rx2*ry+rx2/4;
  while(px<py){_arcPlot(x,y);_arcPlot(-x,y);_arcPlot(x,-y);_arcPlot(-x,-y);
    x++;px+=2*ry2;if(p<0)p+=ry2+px;else{y--;py-=2*rx2;p+=ry2+px-py;}}
  p=ry2*x*x+rx2*(y-1)*(y-1)-rx2*ry2;
  while(y>=0){_arcPlot(x,y);_arcPlot(-x,y);_arcPlot(x,-y);_arcPlot(-x,-y);
    y--;py-=2*rx2;if(p>0)p+=rx2-py;else{x++;px+=2*ry2;p+=rx2-py+px;}}
}
void thickArc(int16_t cx,int16_t cy,int16_t rx,int16_t ry,uint8_t h,uint16_t col){
  drawEllipseArc(cx,cy,rx,ry,h,col);drawEllipseArc(cx,cy,rx,ry+1,h,col);
  drawEllipseArc(cx,cy,rx+1,ry,h,col);drawEllipseArc(cx,cy,rx+1,ry+1,h,col);
}

// ══════════════════════════════════════════════════════════════════════════
//  ERASERS
// ══════════════════════════════════════════════════════════════════════════
void eraseBrow(int16_t x1,int16_t x2,int16_t cy,int16_t s){
  int16_t p=abs(s)+BROW_PAD; tft.fillRectangle(x1-2,cy-p,x2+2,cy+p,C_FACE);
}
void eraseEye(int16_t cx,int16_t cy,int16_t rx,int16_t ry){
  tft.fillRectangle(cx-rx-3,cy-ry-3,cx+rx+3,cy+ry+3,C_FACE);
}
void eraseBlob(int16_t cx,int16_t cy,int16_t r){
  tft.fillRectangle(cx-r-1,cy-r-1,cx+r+1,cy+r+1,C_FACE);
}
void eraseMouth(){
  tft.fillRectangle(MX-M_RX-2,MY-M_RY-6,MX+M_RX+2,MY+M_RY+16,C_FACE);
}
void eraseLabel(){
  tft.fillRectangle(1,LBL_Y-1,174,LBL_Y+LBL_H+1,C_FACE);
}
void eraseZzz(){ tft.fillRectangle(130,30,175,75,C_FACE); }
void clearSweat(){
  eraseBlob(SW_L_X,SW_L_Y,7);eraseBlob(SW_R_X,SW_R_Y,7);
  eraseBlob(SC_L_X,SC_L_Y,6);eraseBlob(SC_R_X,SC_R_Y,6);
}
void eraseNoseArea(){ tft.fillRectangle(NBL-12,NTY-8,NBR+12,NBY+10,C_FACE); }

// ══════════════════════════════════════════════════════════════════════════
//  STATIC HEAD & NOSE
// ══════════════════════════════════════════════════════════════════════════
void drawHead(){
  tft.fillRectangle(0,0,175,219,C_FACE);
  tft.drawRectangle(0,0,175,219,C_OUTLINE);
  tft.fillRectangle(EAR_LX1,EAR_Y1,EAR_LX2,EAR_Y2,C_EAR);
  tft.drawRectangle(EAR_LX1,EAR_Y1,EAR_LX2,EAR_Y2,C_OUTLINE);
  int16_t em=(EAR_Y1+EAR_Y2)/2;
  tft.drawLine(EAR_LX1+1,em,EAR_LX2-1,em,C_OUTLINE);
  tft.fillRectangle(EAR_RX1,EAR_Y1,EAR_RX2,EAR_Y2,C_EAR);
  tft.drawRectangle(EAR_RX1,EAR_Y1,EAR_RX2,EAR_Y2,C_OUTLINE);
  tft.drawLine(EAR_RX1+1,em,EAR_RX2-1,em,C_OUTLINE);
  tft.fillRectangle(84,8,92,22,C_OUTLINE);
  fillEllipse(88,5,8,8,COLOR_YELLOW);
  drawEllipseOutline(88,5,8,8,C_OUTLINE);
}
void drawNose(){
  fillEllipse(NTX,(NTY+NBY)/2,6,(NBY-NTY)/2+2,C_NOSE);
  tft.drawLine(NTX,NTY,NBL,NBY,C_OUTLINE);
  tft.drawLine(NTX,NTY,NBR,NBY,C_OUTLINE);
  tft.drawLine(NBL,NBY,NBR,NBY,C_OUTLINE);
}

// ══════════════════════════════════════════════════════════════════════════
//  BROWS
// ══════════════════════════════════════════════════════════════════════════
void drawLeftBrow(int16_t cy,int16_t s,uint16_t col){
  eraseBrow(LBX1,LBX2,cy,s);
  for(int8_t i=0;i<3;i++) tft.drawLine(LBX1,cy-s+i,LBX2,cy+s+i,col);
}
void drawRightBrow(int16_t cy,int16_t s,uint16_t col){
  eraseBrow(RBX1,RBX2,cy,s);
  for(int8_t i=0;i<3;i++) tft.drawLine(RBX1,cy+s+i,RBX2,cy-s+i,col);
}

// ══════════════════════════════════════════════════════════════════════════
//  EYES
// ══════════════════════════════════════════════════════════════════════════
void drawOneEye(int16_t cx,int16_t cy,int16_t erx,int16_t ery,
                int16_t prx,int16_t pry,int16_t pox,int16_t poy,
                bool closed,int16_t lidPx,bool angryTint){
  eraseEye(cx,cy,erx,ery);
  fillEllipse(cx,cy,erx,ery,C_EYE_W);
  drawEllipseOutline(cx,cy,erx,ery,C_OUTLINE);
  if(closed){
    tft.drawLine(cx-erx,cy,cx+erx,cy,C_OUTLINE);
    tft.drawLine(cx-erx,cy+1,cx+erx,cy+1,C_OUTLINE);
  } else {
    fillEllipse(cx+pox,cy+poy,prx,pry,C_EYE_P);
    drawEllipseOutline(cx+pox,cy+poy,prx,pry,C_OUTLINE);
  }
  if(lidPx>0){
    tft.fillRectangle(cx-erx-1,cy-ery-1,cx+erx+1,cy-ery+lidPx,C_FACE);
    tft.drawLine(cx-erx-1,cy-ery+lidPx,cx+erx+1,cy-ery+lidPx,C_OUTLINE);
  }
  if(angryTint){
    tft.drawRectangle(cx-erx-3,cy-ery-3,cx+erx+3,cy+ery+3,COLOR_RED);
    tft.drawRectangle(cx-erx-2,cy-ery-2,cx+erx+2,cy+ery+2,COLOR_RED);
  }
}
void drawEyes(int16_t pox,int16_t poy,bool closed=false,int16_t lid=0,
              bool angry=false,int16_t erx=ERX,int16_t ery=ERY,
              int16_t prx=PRX,int16_t pry=PRY){
  drawOneEye(LEX,EY,erx,ery,prx,pry,pox,poy,closed,lid,angry);
  drawOneEye(REX,EY,erx,ery,prx,pry,pox,poy,closed,lid,angry);
}
void doBlink(int16_t pox,int16_t poy){
  drawEyes(pox,poy,true); delay(120); drawEyes(pox,poy,false);
}
void movePupils(int16_t nx,int16_t ny){
  pupilOX=nx; pupilOY=ny; drawEyes(pupilOX,pupilOY);
}

// ══════════════════════════════════════════════════════════════════════════
//  MOUTHS – emotion base shapes
// ══════════════════════════════════════════════════════════════════════════
void drawMouth_Happy()   { eraseMouth(); thickArc(MX,MY-10,M_RX,12,0x01,C_LIP); }
void drawMouth_Sad()     { eraseMouth(); thickArc(MX,MY+10,M_RX,12,0x02,C_LIP); }
void drawMouth_Angry()   { eraseMouth(); for(int16_t i=0;i<3;i++) tft.drawLine(MX-28,MY+i,MX+28,MY+i,C_LIP); }
void drawMouth_Surprised(){ eraseMouth(); fillEllipse(MX,MY,16,13,C_BG); drawEllipseOutline(MX,MY,16,13,C_LIP); drawEllipseOutline(MX,MY,15,12,C_LIP); }
void drawMouth_Scared()  { eraseMouth(); for(int16_t x=-30;x<=30;x+=2){ int16_t y=(((x+30)/6)%2==0)?3:-3; tft.drawPixel(MX+x,MY+y,C_LIP); tft.drawPixel(MX+x,MY+y+1,C_LIP); tft.drawPixel(MX+x,MY+y+2,C_LIP); } }
void drawMouth_Disgusted(){ eraseMouth(); tft.drawLine(MX-30,MY-8,MX,MY+4,C_LIP); tft.drawLine(MX,MY+4,MX+30,MY,C_LIP); tft.drawLine(MX-30,MY-7,MX,MY+5,C_LIP); tft.drawLine(MX,MY+5,MX+30,MY+1,C_LIP); }
void drawMouth_Sleepy()  { eraseMouth(); thickArc(MX,MY-5,20,8,0x01,C_LIP); }
void drawMouth_Excited() { eraseMouth(); tft.fillRectangle(MX-M_RX,MY-M_RY+2,MX+M_RX,MY+M_RY-2,C_TEETH); tft.drawRectangle(MX-M_RX,MY-M_RY+2,MX+M_RX,MY+M_RY-2,C_LIP); tft.drawLine(MX,MY-M_RY+2,MX,MY+M_RY-2,C_LIP); }
void drawMouth_Neutral() { eraseMouth(); for(int16_t i=0;i<3;i++) tft.drawLine(MX-24,MY+i,MX+24,MY+i,C_LIP); }

// ── Lip-sync open mouth ───────────────────────────────────────────────────
void drawMouthOpen(uint8_t sz,Emotion emo){
  static const int16_t ryT[5]={0,3,6,10,14};
  static const int16_t rxT[5]={0,8,18,26,33};
  if(sz==0){
    switch(emo){
      case EMO_HAPPY:    drawMouth_Happy();    return;
      case EMO_SAD:      drawMouth_Sad();      return;
      case EMO_ANGRY:    drawMouth_Angry();    return;
      case EMO_DISGUSTED:drawMouth_Disgusted();return;
      case EMO_SLEEPY:   drawMouth_Sleepy();   return;
      case EMO_EXCITED:  drawMouth_Excited();  return;
      default:           drawMouth_Neutral();  return;
    }
  }
  eraseMouth();
  fillEllipse(MX,MY,rxT[sz],ryT[sz],C_INNER);
  drawEllipseOutline(MX,MY,rxT[sz],ryT[sz],C_LIP);
  if(ryT[sz]>=6) tft.drawLine(MX-rxT[sz]+4,MY-ryT[sz]/3,MX+rxT[sz]-4,MY-ryT[sz]/3,C_TEETH);
}

// ══════════════════════════════════════════════════════════════════════════
//  LABEL & ZZZ
// ══════════════════════════════════════════════════════════════════════════
void drawLabel(const char* t,uint16_t col){
  eraseLabel();
  tft.setFont(Terminal6x8);
  int16_t tx=88-((int16_t)strlen(t)*6)/2;
  tft.drawText(tx,LBL_Y,t,col);
}
void drawZzz(){ tft.setFont(Terminal6x8); tft.drawText(138,60,"z",COLOR_CYAN); tft.drawText(150,48,"Z",COLOR_CYAN); tft.drawText(163,35,"Z",COLOR_CYAN); }

// ══════════════════════════════════════════════════════════════════════════
//  APPLY EMOTION
// ══════════════════════════════════════════════════════════════════════════
void applyEmotion(Emotion emo){
  clearSweat(); eraseZzz();
  switch(emo){
    case EMO_HAPPY:
      drawLeftBrow(BROW_CY-4,-5,C_OUTLINE); drawRightBrow(BROW_CY-4,-5,C_OUTLINE);
      drawEyes(0,3); drawMouth_Happy(); drawLabel("HAPPY",COLOR_YELLOW); break;
    case EMO_SAD:
      drawLeftBrow(BROW_CY+3,6,C_OUTLINE); drawRightBrow(BROW_CY+3,6,C_OUTLINE);
      drawEyes(0,3); drawMouth_Sad();
      fillEllipse(SW_L_X,SW_L_Y,4,5,C_SWEAT); fillEllipse(SW_R_X,SW_R_Y,4,5,C_SWEAT);
      drawLabel("SAD",0x07FF); break;
    case EMO_ANGRY:
      drawLeftBrow(BROW_CY+2,7,COLOR_RED); drawRightBrow(BROW_CY+2,7,COLOR_RED);
      drawEyes(0,4,false,0,true); drawMouth_Angry(); drawLabel("ANGRY",COLOR_RED); break;
    case EMO_SURPRISED:
      drawLeftBrow(BROW_CY-8,-7,C_OUTLINE); drawRightBrow(BROW_CY-8,-7,C_OUTLINE);
      drawEyes(0,0,false,0,false,ERX+3,ERY+4,PRX+1,PRY+1);
      drawMouth_Surprised(); drawLabel("SURPRISED",COLOR_YELLOW); break;
    case EMO_SCARED:
      drawLeftBrow(BROW_CY-6,-6,C_OUTLINE); drawRightBrow(BROW_CY-6,-6,C_OUTLINE);
      drawEyes(4,-4,false,0,false,ERX+3,ERY+4,PRX-1,PRY-1);
      drawMouth_Scared();
      fillEllipse(SC_L_X,SC_L_Y,4,5,C_SWEAT); fillEllipse(SC_R_X,SC_R_Y,4,5,C_SWEAT);
      drawLabel("SCARED",0x07FF); break;
    case EMO_DISGUSTED:
      drawLeftBrow(BROW_CY+3,4,C_OUTLINE); drawRightBrow(BROW_CY-1,-3,C_OUTLINE);
      drawOneEye(LEX,EY,ERX,ERY,PRX,PRY,-3,5,false,ERY,false);
      drawOneEye(REX,EY,ERX,ERY,PRX,PRY,0,2,false,0,false);
      drawMouth_Disgusted(); drawLabel("DISGUSTED",COLOR_GREEN); break;
    case EMO_SLEEPY:
      drawLeftBrow(BROW_CY+2,0,C_OUTLINE); drawRightBrow(BROW_CY+2,0,C_OUTLINE);
      drawEyes(0,5,false,ERY); drawMouth_Sleepy(); drawZzz(); drawLabel("SLEEPY",0x7BEF); break;
    case EMO_EXCITED:
      drawLeftBrow(BROW_CY-6,-8,COLOR_YELLOW); drawRightBrow(BROW_CY-6,-8,COLOR_YELLOW);
      drawEyes(0,0,false,0,false,ERX,ERY,PRX+3,PRY+3);
      { const int16_t sc[2]={LEX,REX};
        for(uint8_t i=0;i<2;i++){
          tft.drawLine(sc[i]-13,EY,sc[i]+13,EY,COLOR_YELLOW);
          tft.drawLine(sc[i],EY-13,sc[i],EY+13,COLOR_YELLOW);
          tft.drawLine(sc[i]-9,EY-9,sc[i]+9,EY+9,COLOR_YELLOW);
          tft.drawLine(sc[i]-9,EY+9,sc[i]+9,EY-9,COLOR_YELLOW);
        }
      }
      drawMouth_Excited(); drawLabel("EXCITED",COLOR_YELLOW); break;
    default:
      drawLeftBrow(BROW_CY,0,C_OUTLINE); drawRightBrow(BROW_CY,0,C_OUTLINE);
      drawEyes(0,0); drawMouth_Neutral(); drawLabel("LISTENING",COLOR_WHITE); break;
  }
  pupilOX=0; pupilOY=0;
}

// ══════════════════════════════════════════════════════════════════════════
//  COMMAND: SHOW TONGUE
// ══════════════════════════════════════════════════════════════════════════
void cmdShowTongue(){
  drawLeftBrow(BROW_CY-4,-5,C_OUTLINE); drawRightBrow(BROW_CY-4,-5,C_OUTLINE);
  drawEyes(0,3);
  eraseMouth();
  fillEllipse(MX,MY,M_RX-2,M_RY-2,C_INNER);
  drawEllipseOutline(MX,MY,M_RX-2,M_RY-2,C_LIP);
  tft.drawLine(MX-28,MY-M_RY+6,MX+28,MY-M_RY+6,C_TEETH);
  tft.drawLine(MX-28,MY-M_RY+7,MX+28,MY-M_RY+7,C_TEETH);
  // tongue hanging below mouth
  fillEllipse(MX,MY+M_RY+5,14,10,C_TONGUE);
  drawEllipseOutline(MX,MY+M_RY+5,14,10,C_LIP);
  tft.drawLine(MX,MY+M_RY,MX,MY+M_RY+13,0xF800);  // centre groove
  drawLabel("TONGUE!",COLOR_YELLOW);
  delay(2500); applyEmotion(EMO_NEUTRAL);
}

// ══════════════════════════════════════════════════════════════════════════
//  COMMAND: SHOW TEETH
// ══════════════════════════════════════════════════════════════════════════
void cmdShowTeeth(){
  drawLeftBrow(BROW_CY-4,-5,COLOR_YELLOW); drawRightBrow(BROW_CY-4,-5,COLOR_YELLOW);
  drawEyes(0,2);
  eraseMouth();
  tft.fillRectangle(MX-M_RX+2,MY-M_RY+2,MX+M_RX-2,MY+M_RY-2,C_TEETH);
  tft.drawRectangle(MX-M_RX+2,MY-M_RY+2,MX+M_RX-2,MY+M_RY-2,C_LIP);
  for(int16_t i=1;i<5;i++){
    int16_t tx=MX-M_RX+2+i*13;
    tft.drawLine(tx,MY-M_RY+2,tx,MY,C_LIP);
    tft.drawLine(tx,MY,tx,MY+M_RY-2,C_LIP);
  }
  tft.drawLine(MX-M_RX+2,MY,MX+M_RX-2,MY,C_LIP);
  drawLabel("TEETH!",COLOR_WHITE);
  delay(2500); applyEmotion(EMO_NEUTRAL);
}

// ══════════════════════════════════════════════════════════════════════════
//  COMMAND: SHOW EYES
// ══════════════════════════════════════════════════════════════════════════
void cmdShowEyes(){
  drawLeftBrow(BROW_CY-8,-7,C_OUTLINE); drawRightBrow(BROW_CY-8,-7,C_OUTLINE);
  eraseMouth(); drawMouth_Surprised();
  drawOneEye(LEX,EY,ERX+5,ERY+6,PRX+4,PRY+4,0,0,false,0,false);
  drawOneEye(REX,EY,ERX+5,ERY+6,PRX+4,PRY+4,0,0,false,0,false);
  // glint highlights
  tft.drawPixel(LEX-5,EY-5,COLOR_WHITE); tft.drawPixel(LEX-4,EY-5,COLOR_WHITE);
  tft.drawPixel(LEX-5,EY-4,COLOR_WHITE);
  tft.drawPixel(REX-5,EY-5,COLOR_WHITE); tft.drawPixel(REX-4,EY-5,COLOR_WHITE);
  tft.drawPixel(REX-5,EY-4,COLOR_WHITE);
  drawLabel("EYES!",COLOR_CYAN);
  delay(2500); applyEmotion(EMO_NEUTRAL);
}

// ══════════════════════════════════════════════════════════════════════════
//  COMMAND: SHOW NOSE
// ══════════════════════════════════════════════════════════════════════════
void cmdShowNose(){
  drawLeftBrow(BROW_CY,0,C_OUTLINE); drawRightBrow(BROW_CY,0,C_OUTLINE);
  drawEyes(0,0); drawMouth_Neutral();
  eraseNoseArea();
  fillEllipse(NTX,(NTY+NBY)/2,8,(NBY-NTY)/2+4,COLOR_YELLOW);
  tft.drawLine(NTX,NTY-4,NBL-8,NBY+4,COLOR_YELLOW);
  tft.drawLine(NTX,NTY-4,NBR+8,NBY+4,COLOR_YELLOW);
  tft.drawLine(NBL-8,NBY+4,NBR+8,NBY+4,COLOR_YELLOW);
  // arrow pointers
  tft.drawLine(30,136,58,136,COLOR_RED);
  tft.drawLine(30,136,38,130,COLOR_RED); tft.drawLine(30,136,38,142,COLOR_RED);
  tft.drawLine(118,136,146,136,COLOR_RED);
  tft.drawLine(138,130,146,136,COLOR_RED); tft.drawLine(138,142,146,136,COLOR_RED);
  drawLabel("NOSE!",COLOR_YELLOW);
  delay(2500); drawNose(); applyEmotion(EMO_NEUTRAL);
}

// ══════════════════════════════════════════════════════════════════════════
//  COMMAND: SHOW LIPS
// ══════════════════════════════════════════════════════════════════════════
void cmdShowLips(){
  drawLeftBrow(BROW_CY-3,-4,C_OUTLINE); drawRightBrow(BROW_CY-3,-4,C_OUTLINE);
  drawEyes(0,2);
  eraseMouth();
  fillEllipse(MX,MY-6,M_RX-4,7,C_LIP);
  fillEllipse(MX,MY+7,M_RX-4,7,0xF800);
  drawEllipseOutline(MX,MY-6,M_RX-4,7,0xB000);
  drawEllipseOutline(MX,MY+7,M_RX-4,7,0xB000);
  tft.drawLine(MX-18,MY-6,MX,MY-11,C_OUTLINE);
  tft.drawLine(MX,MY-11,MX+18,MY-6,C_OUTLINE);
  drawLabel("LIPS!",COLOR_RED);
  delay(2500); applyEmotion(EMO_NEUTRAL);
}

// ══════════════════════════════════════════════════════════════════════════
//  COMMAND: KISS  (pucker + floating hearts)
// ══════════════════════════════════════════════════════════════════════════
void drawHeart(int16_t cx,int16_t cy,int16_t s,uint16_t col){
  fillEllipse(cx-s/2,cy-s/4,s/2,s/3,col);
  fillEllipse(cx+s/2,cy-s/4,s/2,s/3,col);
  for(int16_t y=0;y<s;y++){
    int16_t hw=(s-y)*s/(s+1);
    tft.drawLine(cx-hw/2,cy+y/2,cx+hw/2,cy+y/2,col);
  }
}
void cmdKiss(){
  drawLeftBrow(BROW_CY-4,-5,C_OUTLINE); drawRightBrow(BROW_CY-4,-5,C_OUTLINE);
  drawOneEye(LEX,EY,ERX,ERY,PRX,PRY,0,2,false,ERY/2,false);
  drawOneEye(REX,EY,ERX,ERY,PRX,PRY,0,2,false,ERY/2,false);
  eraseMouth();
  fillEllipse(MX,MY,10,8,C_LIP);
  drawEllipseOutline(MX,MY,10,8,0xB000);
  fillEllipse(MX,MY,5,4,0xB000);
  drawLabel("KISS! <3",C_KISS);
  for(uint8_t i=0;i<4;i++){
    int16_t hx=MX+(int16_t)(random(80))-40;
    int16_t hy=MY-20;
    for(uint8_t step=0;step<5;step++){
      drawHeart(hx,hy-step*9,8,C_HEART);
      delay(100);
      drawHeart(hx,hy-step*9,8,C_FACE);
    }
  }
  delay(800); applyEmotion(EMO_NEUTRAL);
}

// ══════════════════════════════════════════════════════════════════════════
//  COMMAND: EYE DIRECTIONS
// ══════════════════════════════════════════════════════════════════════════
void cmdSeeLeft(){
  drawLeftBrow(BROW_CY,0,C_OUTLINE); drawRightBrow(BROW_CY,0,C_OUTLINE);
  drawEyes(-(ERX-4),0); drawMouth_Neutral(); drawLabel("SEE LEFT",COLOR_CYAN);
  delay(2000); applyEmotion(EMO_NEUTRAL);
}
void cmdSeeRight(){
  drawLeftBrow(BROW_CY,0,C_OUTLINE); drawRightBrow(BROW_CY,0,C_OUTLINE);
  drawEyes(ERX-4,0); drawMouth_Neutral(); drawLabel("SEE RIGHT",COLOR_CYAN);
  delay(2000); applyEmotion(EMO_NEUTRAL);
}
void cmdSeeUp(){
  drawLeftBrow(BROW_CY-6,-5,C_OUTLINE); drawRightBrow(BROW_CY-6,-5,C_OUTLINE);
  drawEyes(0,-(ERY-4)); drawMouth_Surprised(); drawLabel("SEE UP",COLOR_CYAN);
  delay(2000); applyEmotion(EMO_NEUTRAL);
}
void cmdSeeDown(){
  drawLeftBrow(BROW_CY+3,3,C_OUTLINE); drawRightBrow(BROW_CY+3,3,C_OUTLINE);
  drawEyes(0,ERY-3); drawMouth_Neutral(); drawLabel("SEE DOWN",COLOR_CYAN);
  delay(2000); applyEmotion(EMO_NEUTRAL);
}
void cmdSeeBack(){
  drawLeftBrow(BROW_CY-2,-3,C_OUTLINE); drawRightBrow(BROW_CY+4,5,C_OUTLINE);
  drawEyes(0,0,true); drawMouth_Neutral(); drawLabel("CAN'T SEE!",0x7BEF);
  delay(2000); applyEmotion(EMO_NEUTRAL);
}
void cmdSeeFront(){
  drawLeftBrow(BROW_CY,0,C_OUTLINE); drawRightBrow(BROW_CY,0,C_OUTLINE);
  drawEyes(0,0); drawMouth_Happy(); drawLabel("SEE FRONT!",COLOR_GREEN);
  delay(1500); applyEmotion(EMO_NEUTRAL);
}

// ══════════════════════════════════════════════════════════════════════════
//  COMMAND: ARE YOU HUNGRY  (drooly open mouth)
// ══════════════════════════════════════════════════════════════════════════
void cmdHungry(){
  drawLeftBrow(BROW_CY-5,-6,COLOR_YELLOW); drawRightBrow(BROW_CY-5,-6,COLOR_YELLOW);
  drawEyes(0,2);
  eraseMouth();
  fillEllipse(MX,MY,M_RX-4,M_RY-2,C_INNER);
  drawEllipseOutline(MX,MY,M_RX-4,M_RY-2,C_LIP);
  tft.drawLine(MX-25,MY-M_RY+6,MX+25,MY-M_RY+6,C_TEETH);
  // drool streams
  for(uint8_t d=0;d<3;d++){
    tft.drawLine(MX-8,MY+M_RY-2,MX-8,MY+M_RY+10+d*4,C_SWEAT);
    fillEllipse(MX-8,MY+M_RY+13+d*4,4,4,C_SWEAT);
    tft.drawLine(MX+8,MY+M_RY-2,MX+8,MY+M_RY+8+d*4,C_SWEAT);
    fillEllipse(MX+8,MY+M_RY+11+d*4,3,4,C_SWEAT);
    delay(400);
  }
  drawLabel("SO HUNGRY!",COLOR_YELLOW);
  delay(1500); applyEmotion(EMO_NEUTRAL);
}

// ══════════════════════════════════════════════════════════════════════════
//  COMMAND: SAY RHYMES – Twinkle Twinkle full-screen bottom-to-top scroll
// ══════════════════════════════════════════════════════════════════════════
const char rl00[] PROGMEM = "~ Twinkle Twinkle ~";
const char rl01[] PROGMEM = "  little star,  ";
const char rl02[] PROGMEM = "How I wonder";
const char rl03[] PROGMEM = "what you are!";
const char rl04[] PROGMEM = "Up above the world";
const char rl05[] PROGMEM = "  so high,";
const char rl06[] PROGMEM = "Like a diamond";
const char rl07[] PROGMEM = "  in the sky.";
const char rl08[] PROGMEM = "~ Twinkle Twinkle ~";
const char rl09[] PROGMEM = "  little star,";
const char rl10[] PROGMEM = "How I wonder";
const char rl11[] PROGMEM = "what you are!";
const char rl12[] PROGMEM = "When the blazing";
const char rl13[] PROGMEM = "  sun is gone,";
const char rl14[] PROGMEM = "When he nothing";
const char rl15[] PROGMEM = "  shines upon,";
const char rl16[] PROGMEM = "Then you show your";
const char rl17[] PROGMEM = "  little light,";
const char rl18[] PROGMEM = "Twinkle twinkle";
const char rl19[] PROGMEM = "  all the night!";
const char rl20[] PROGMEM = "~ Twinkle Twinkle ~";
const char rl21[] PROGMEM = "  little star,";
const char rl22[] PROGMEM = "How I wonder";
const char rl23[] PROGMEM = "what you are!";
const char rl24[] PROGMEM = "";
const char rl25[] PROGMEM = "  *** THE END ***";

const char* const rhymeLines[] PROGMEM = {
  rl00,rl01,rl02,rl03,rl04,rl05,rl06,rl07,
  rl08,rl09,rl10,rl11,rl12,rl13,rl14,rl15,
  rl16,rl17,rl18,rl19,rl20,rl21,rl22,rl23,rl24,rl25
};
#define RHYME_LINES 26
#define LINE_H       14

static const uint8_t starX[]={15,155,8,162,25,148,78,102,48,130,68,108};
static const uint8_t starY[]={18, 16,48,  44,78,  74, 8, 13,28,  26,58,  53};
#define N_STARS 12
bool starState[N_STARS]={0};
void drawStar(uint8_t i,bool on){
  uint16_t c=on?COLOR_YELLOW:C_BG;
  tft.drawPixel(starX[i],starY[i],c);   tft.drawPixel(starX[i]+1,starY[i],c);
  tft.drawPixel(starX[i],starY[i]+1,c); tft.drawPixel(starX[i]+1,starY[i]+1,c);
}

void cmdSayRhymes(){
  tft.clear();
  tft.fillRectangle(0,0,175,219,C_BG);

  int16_t scrollY = 220;
  uint32_t lastStar=0;
  uint8_t  starIdx=0;
  char lineBuf[24];

  while(scrollY > -(int16_t)(RHYME_LINES*LINE_H)){
    scrollY -= 1;   // 1 px per tick = smooth scroll

    for(uint8_t li=0;li<RHYME_LINES;li++){
      int16_t lineY=scrollY+li*LINE_H;
      if(lineY<-LINE_H||lineY>219) continue;
      strcpy_P(lineBuf,(char*)pgm_read_word(&(rhymeLines[li])));

      uint16_t lc;
      switch((li/2)%5){
        case 0: lc=COLOR_YELLOW; break;
        case 1: lc=COLOR_WHITE;  break;
        case 2: lc=0x07FF;       break;
        case 3: lc=COLOR_GREEN;  break;
        default:lc=0xFF00;       break;
      }
      tft.fillRectangle(0,lineY,175,lineY+LINE_H-1,C_BG);
      tft.setFont(Terminal6x8);
      int16_t tx=88-((int16_t)strlen(lineBuf)*6)/2;
      tft.drawText(tx,lineY,lineBuf,lc);
    }

    // Twinkle stars
    if(millis()-lastStar>100){
      drawStar(starIdx,starState[starIdx]);
      starState[starIdx]=!starState[starIdx];
      starIdx=(starIdx+1)%N_STARS;
      lastStar=millis();
    }

    delay(28);
  }

  // Restore robot face
  drawHead(); drawNose();
  applyEmotion(EMO_HAPPY); delay(1000);
  applyEmotion(EMO_NEUTRAL);
}

// ══════════════════════════════════════════════════════════════════════════
//  COMMAND: MATH  – parse a OP b, show result big on face
// ══════════════════════════════════════════════════════════════════════════
bool tryMath(const char* msg, long& result){
  const char* p=msg;
  while(*p==' ')p++;
  if(!isdigit(*p)&&!(*p=='-'&&isdigit(*(p+1)))) return false;
  long a=atol(p);
  while(*p&&(isdigit(*p)||(*p=='-'&&p==msg)))p++;
  while(*p==' ')p++;
  if(!*p) return false;
  char op=*p; p++;
  while(*p==' ')p++;
  if(!isdigit(*p)&&!(*p=='-'&&isdigit(*(p+1)))) return false;
  long b=atol(p);
  switch(op){
    case '+': result=a+b; return true;
    case '-': result=a-b; return true;
    case '*': case 'x': case 'X': result=a*b; return true;
    case '/': if(b==0)return false; result=a/b; return true;
  }
  return false;
}

void cmdMathResult(long res){
  drawLeftBrow(BROW_CY-6,-8,COLOR_YELLOW); drawRightBrow(BROW_CY-6,-8,COLOR_YELLOW);
  drawEyes(0,0,false,0,false,ERX,ERY,PRX+3,PRY+3);
  eraseMouth(); drawMouth_Excited();
  // Clear nose area and show big number
  tft.fillRectangle(8,108,168,162,C_BG);
  char buf[16];
  ltoa(res,buf,10);
  tft.setFont(Terminal12x16);
  int16_t tx=88-((int16_t)strlen(buf)*12)/2;
  tft.drawText(tx,120,buf,COLOR_YELLOW);
  tft.setFont(Terminal6x8);
  tft.drawText(77,108,"=",COLOR_WHITE);
  drawLabel("ANSWER!",COLOR_YELLOW);
  for(uint8_t i=0;i<4;i++){delay(300);doBlink(0,0);}
  delay(2000);
  tft.fillRectangle(8,105,168,165,C_FACE);
  drawNose(); applyEmotion(EMO_NEUTRAL);
}

// ══════════════════════════════════════════════════════════════════════════
//  HELPERS
// ══════════════════════════════════════════════════════════════════════════
bool strContains(const char* h,const char* n){
  uint8_t hl=strlen(h),nl=strlen(n);
  if(nl>hl)return false;
  for(uint8_t i=0;i<=hl-nl;i++){
    bool m=true;
    for(uint8_t j=0;j<nl;j++){
      if(tolower((uint8_t)h[i+j])!=tolower((uint8_t)n[j])){m=false;break;}
    }
    if(m)return true;
  }
  return false;
}
Emotion detectEmotion(const char* m){
  if(strContains(m,"hi")||strContains(m,"hello")||strContains(m,"haha")||
     strContains(m,"happy")||strContains(m,"good")||strContains(m,"great")||
     strContains(m,"awesome")||strContains(m,"love")||strContains(m,"yay")||
     strContains(m,":)")||strContains(m,"nice"))return EMO_HAPPY;
  if(strContains(m,"wow")||strContains(m,"omg")||strContains(m,"amazing")||
     strContains(m,"excited")||strContains(m,"fantastic")||strContains(m,"incredible"))return EMO_EXCITED;
  if(strContains(m,"really")||strContains(m,"seriously")||strContains(m,"no way")||
     strContains(m,"surprised")||strContains(m,"whoa"))return EMO_SURPRISED;
  if(strContains(m,"angry")||strContains(m,"mad")||strContains(m,"hate")||
     strContains(m,"stupid")||strContains(m,"rage"))return EMO_ANGRY;
  if(strContains(m,"sad")||strContains(m,"cry")||strContains(m,"miss")||
     strContains(m,"sorry")||strContains(m,"lonely")||strContains(m,"depress")||
     strContains(m,"unhappy")||strContains(m,"hurt"))return EMO_SAD;
  if(strContains(m,"scared")||strContains(m,"fear")||strContains(m,"afraid")||
     strContains(m,"dark")||strContains(m,"monster"))return EMO_SCARED;
  if(strContains(m,"ew")||strContains(m,"gross")||strContains(m,"yuck")||
     strContains(m,"disgust")||strContains(m,"awful"))return EMO_DISGUSTED;
  if(strContains(m,"sleep")||strContains(m,"tired")||strContains(m,"bore")||
     strContains(m,"yawn")||strContains(m,"night")||strContains(m,"bed")||
     strContains(m,"zzz"))return EMO_SLEEPY;
  return EMO_NEUTRAL;
}

// ══════════════════════════════════════════════════════════════════════════
//  LIP-SYNC
// ══════════════════════════════════════════════════════════════════════════
uint8_t charToOpenSize(char c){
  c=tolower(c);
  switch(c){
    case 'a':case 'o':return 4;
    case 'e':case 'u':return 3;
    case 'i':return 2;
    case 'm':case 'b':case 'p':return 0;
    case ' ':case ',':case '.':return 1;
    default:return 2;
  }
}
static const int8_t eyeSeq[][2]={{0,0},{4,0},{-4,0},{0,2},{3,-2},{-3,0},{0,0},{2,2},{-2,-1}};
void lipSync(const char* msg,Emotion emo){
  uint8_t len=strlen(msg),prevOpen=0,eyeStep=0;
  for(uint8_t i=0;i<len;i++){
    char c=msg[i];
    uint8_t sz=charToOpenSize(c);
    if(sz!=prevOpen){drawMouthOpen(sz,emo);prevOpen=sz;}
    if((i%4)==0){
      uint8_t idx=eyeStep%9;
      if(emo!=EMO_DISGUSTED&&emo!=EMO_SCARED)
        drawEyes(eyeSeq[idx][0],eyeSeq[idx][1],false,0,emo==EMO_ANGRY,
                 (emo==EMO_SURPRISED)?ERX+3:ERX,(emo==EMO_SURPRISED)?ERY+4:ERY);
      eyeStep++;
    }
    uint16_t d=(c==' '||c==','||c=='.')?150:(sz>=3?110:(sz==0?70:90));
    delay(d);
    if(millis()-lastBlink>blinkInt){doBlink(pupilOX,pupilOY);lastBlink=millis();blinkInt=3000+random(1500);}
  }
  drawMouthOpen(0,emo);
}

// ══════════════════════════════════════════════════════════════════════════
//  SETUP & LOOP
// ══════════════════════════════════════════════════════════════════════════
#define MAX_MSG 80
char msgBuf[MAX_MSG+1];
uint8_t msgLen=0;

void setup(){
#if defined(ESP32)
  hspi.begin(); tft.begin(hspi);
#else
  tft.begin();
#endif
  tft.setOrientation(0);
  tft.setBacklight(true);
  tft.clear();
  Serial.begin(9600);
  drawHead(); drawNose();
  applyEmotion(EMO_NEUTRAL);
  lastBlink=millis(); blinkInt=3000;
  Serial.println(F("=== Robot v6 Ready ==="));
  Serial.println(F("show tongue | show teeth | show eyes | show nose | show lips"));
  Serial.println(F("kiss | see left/right/up/down/back/front"));
  Serial.println(F("are you hungry | say rhymes"));
  Serial.println(F("math: 3+4  5*6  10-2  20/4"));
  Serial.println(F("emotions: hi hello sad angry wow ew sleepy ..."));
}

void loop(){
  while(Serial.available()){
    char ch=(char)Serial.read();
    if(ch=='\n'||ch=='\r'){
      if(msgLen>0){
        msgBuf[msgLen]='\0'; msgLen=0;
        // Priority 1: special commands
        if(strContains(msgBuf,"tongue")){cmdShowTongue();return;}
        if(strContains(msgBuf,"teeth")){cmdShowTeeth();return;}
        if(strContains(msgBuf,"show")&&strContains(msgBuf,"eye")){cmdShowEyes();return;}
        if(strContains(msgBuf,"show")&&strContains(msgBuf,"nose")){cmdShowNose();return;}
        if(strContains(msgBuf,"show")&&strContains(msgBuf,"lip")){cmdShowLips();return;}
        if(strContains(msgBuf,"kiss")){cmdKiss();return;}
        if(strContains(msgBuf,"see left")||strContains(msgBuf,"look left")){cmdSeeLeft();return;}
        if(strContains(msgBuf,"see right")||strContains(msgBuf,"look right")){cmdSeeRight();return;}
        if(strContains(msgBuf,"see up")||strContains(msgBuf,"look up")){cmdSeeUp();return;}
        if(strContains(msgBuf,"see down")||strContains(msgBuf,"look down")){cmdSeeDown();return;}
        if(strContains(msgBuf,"see back")||strContains(msgBuf,"look back")||strContains(msgBuf,"look away")){cmdSeeBack();return;}
        if(strContains(msgBuf,"see front")||strContains(msgBuf,"look front")||strContains(msgBuf,"look here")){cmdSeeFront();return;}
        if(strContains(msgBuf,"hungry")||strContains(msgBuf,"food")||strContains(msgBuf,"eat")){cmdHungry();return;}
        if(strContains(msgBuf,"rhyme")||strContains(msgBuf,"twinkle")||strContains(msgBuf,"sing")||strContains(msgBuf,"poem")){cmdSayRhymes();return;}
        // Priority 2: math
        long mathResult=0;
        if(tryMath(msgBuf,mathResult)){cmdMathResult(mathResult);return;}
        // Priority 3: emotion + lip-sync
        Emotion emo=detectEmotion(msgBuf);
        currentEmo=emo;
        applyEmotion(emo); delay(300);
        doBlink(0,0); delay(100);
        isTalking=true; lipSync(msgBuf,emo); isTalking=false;
        delay(1500); applyEmotion(EMO_NEUTRAL); drawLabel("LISTENING",COLOR_WHITE);
      }
    } else {
      if(msgLen<MAX_MSG) msgBuf[msgLen++]=ch;
    }
  }
  // Idle
  if(!isTalking){
    uint32_t now=millis();
    if(now-lastBlink>blinkInt){
      doBlink(pupilOX,pupilOY); lastBlink=now; blinkInt=2800+random(2000);
      movePupils((int8_t)(random(7))-3,(int8_t)(random(5))-2);
    }
  }
}
