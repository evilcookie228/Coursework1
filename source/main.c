#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "topscreenbackground.h"

typedef enum {
    ELEMENT_NONE,
    START_EVENT,
    END_EVENT,
    MID_EVENT,
    TASK,
    GATEWAY,
    FLOW
} BPMNElementType ;

// Structure for BPMN elements
typedef struct {
    BPMNElementType type;
    int startrow;
    int startcolumn;
    int endrow;
    int endcolumn;
} BPMNElement;

bool arrowdrawn = false;
int arrowrow = 0;
int arrowcolumn = 0;

#define DISPLAY_WIDTH 256
#define DISPLAY_HEIGHT 192

#define ROWS 20
#define COLUMNS 50
#define SIZE_X 16
#define SIZE_Y 16
#define GREEN RGB15(0, 31, 0)
#define RED RGB15(31, 0, 0)
#define WHITE RGB15(31, 31, 31)
#define BLACK RGB15(0, 0, 0)
#define GRAY RGB15(15, 15, 15)
#define BLUE RGB15(0, 0, 31)

#define REDFANCY ARGB16(1, 31, 0,0 )
#define GREENFANCY ARGB16(1, 0, 31, 0)
#define GRAYFANCY ARGB16(1, 15, 15, 15)
#define BLACKFANCY ARGB16(1, 0, 0, 0)
#define WHITEFANCY ARGB16(1, 31, 31, 31)
#define BLUEFANCY ARGB16(1, 0, 0, 31)

BPMNElement elements[ROWS][COLUMNS];
BPMNElement flows[64];
int totalflows = 0;
BPMNElementType currentTool = TASK;
int canvasOffsetX = 0;
int canvasOffsetY = 0;
int topScreenCursor = 0;



void drawCursor(u16* cursor1)
{
    switch (topScreenCursor)
    {
        case 1:
            oamSet(&oamMain, 3, 115, 60, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
            break;
        case 2:
            oamSet(&oamMain, 3, 190, 60, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
            break;
        case 3:
            oamSet(&oamMain, 3, 35, 135, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
            break;
        case 4:
            oamSet(&oamMain, 3, 95, 135, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
            break;
        case 5:
            oamSet(&oamMain, 3, 160, 135, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
            break;
        case 6:
            oamSet(&oamMain, 3, 210, 135, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
            break;
        default:
            oamSet(&oamMain, 3, 45, 60, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
    }
    oamUpdate(&oamMain);
}

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

void drawDiamond(int x, int y, u16 color) {
    // Diamond will be 14x14 to fit nicely in 16x16 with border space
    const int size = 14;
    const int half = size / 2;
    
    // Center in the 16x16 cell (offset by 1 pixel on each side)
    x += 1;
    y += 1;
    
    for (int dy = 0; dy < size; dy++) {
        int width = half - abs(dy - half);
        for (int dx = half - width; dx <= half + width; dx++) {
            int px = x + dx;
            int py = y + dy;
            
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                BG_GFX_SUB[py * SCREEN_WIDTH + px] = color;
            }
        }
    }
}

void drawRectangle(int x, int y, u16 color) {
    // Rectangle will be 12x8 to match BPMN task aspect ratio
    const int width = 12;
    const int height = 8;
    
    // Center in the 16x16 cell
    x += 2;  // (16-12)/2 = 2
    y += 4;  // (16-8)/2 = 4
    
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int px = x + dx;
            int py = y + dy;
            
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                BG_GFX_SUB[py * SCREEN_WIDTH + px] = color;
            }
        }
    }
    
    // Add rounded corners (optional)
    BG_GFX_SUB[(y) * SCREEN_WIDTH + (x)] = GRAYFANCY;
    BG_GFX_SUB[(y) * SCREEN_WIDTH + (x + width - 1)] = GRAYFANCY;
    BG_GFX_SUB[(y + height - 1) * SCREEN_WIDTH + (x)] = GRAYFANCY;
    BG_GFX_SUB[(y + height - 1) * SCREEN_WIDTH + (x + width - 1)] = GRAYFANCY;
}

void drawArrow(BPMNElement Arrow, u16 color)
{
    int x1 = Arrow.startcolumn;
    int y1 = Arrow.startrow;
    int x2 = Arrow.endcolumn;
    int y2 = Arrow.endrow;

    if (x1 - canvasOffsetX >= 16) {x1 = 15;}
    if (x2 - canvasOffsetX >= 16) {x2 = 15;}
    if (y1 - canvasOffsetX >= 12) {x1 = 11;}
    if (y2 - canvasOffsetX >= 12) {x2 = 11;}

    x1 *= 16;
    x2 *= 16;
    y1 *= 16;
    y2 *= 16;
    // Draw line using Bresenham's algorithm
    
    
    for (int i = MIN(x1, x2); i < MAX(x1, x2); i++)
    {
        BG_GFX_SUB[(y1 + 8) * SCREEN_WIDTH + i + 8] = color;
        BG_GFX_SUB[(y1 + 9) * SCREEN_WIDTH + i + 8] = color;
    }

    for (int i = MIN(y1, y2); i < MAX(y1, y2), i++)
    {
        BG_GFX_SUB[(i + 8) * SCREEN_WIDTH + x1 + 8] = color;
        BG_GFX_SUB[(i + 8) * SCREEN_WIDTH + x1 + 9] = color;
    }
}


void drawCircle(int xc, int yc, u16 color) {
    const int radius = 7; // For 16x16 circle (diameter 15)
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        // Draw horizontal lines to fill the circle
        for (int dx = -x; dx <= x; dx++) {
            int px1 = xc + 8 + dx; // +8 to center in 16x16
            int py1 = yc + 8 + y;  // +8 to center in 16x16
            int py2 = yc + 8 - y;
            
            if (px1 >= 0 && px1 < SCREEN_WIDTH) {
                if (py1 >= 0 && py1 < SCREEN_HEIGHT) {
                    BG_GFX_SUB[py1 * SCREEN_WIDTH + px1] = color;
                }
                if (py2 >= 0 && py2 < SCREEN_HEIGHT) {
                    BG_GFX_SUB[py2 * SCREEN_WIDTH + px1] = color;
                }
            }
        }
        
        for (int dx = -y; dx <= y; dx++) {
            int px2 = xc + 8 + dx;
            int py3 = yc + 8 + x;
            int py4 = yc + 8 - x;
            
            if (px2 >= 0 && px2 < SCREEN_WIDTH) {
                if (py3 >= 0 && py3 < SCREEN_HEIGHT) {
                    BG_GFX_SUB[py3 * SCREEN_WIDTH + px2] = color;
                }
                if (py4 >= 0 && py4 < SCREEN_HEIGHT) {
                    BG_GFX_SUB[py4 * SCREEN_WIDTH + px2] = color;
                }
            }
        }

        y += 1;
        err += 1 + 2*y;
        if (2*(err - x) + 1 > 0) {
            x -= 1;
            err += 1 - 2*x;
        }
    }
    }


void resetScreen()
{
    for (int i = 0; i < 192; i++)
    {
        for (int j = 0; j < 256; j++)
        {
            BG_GFX_SUB[i * 256 + j] = GRAYFANCY;
        }
    }
    return;
}

void highlightGrid(int x, int y)
{
    for (int dy = 0; dy < SIZE_Y; dy++) 
    {
        for (int dx = 0; dx < SIZE_X; dx++) 
        {
            BG_GFX_SUB[DISPLAY_WIDTH  * (y * SIZE_Y + dy) + (x * SIZE_X + dx)] = REDFANCY;
        }
    }
    return;
}

void addFigure(int x, int y)
{
    BPMNElement tempelement;
    switch (topScreenCursor)
    {
        case 1:
            tempelement.type = END_EVENT;
            tempelement.startrow = -1;
            tempelement.startcolumn = -1;
            tempelement.endrow = -1;
            tempelement.endcolumn = -1;
            elements[y][x] = tempelement;
            break;
        case 2:
            tempelement.type = MID_EVENT;
            tempelement.startrow = -1;
            tempelement.startcolumn = -1;
            tempelement.endrow = -1;
            tempelement.endcolumn = -1;
            elements[y][x] = tempelement;
            break;  
        case 3:
            tempelement.type = GATEWAY;
            tempelement.startrow = -1;
            tempelement.startcolumn = -1;
            tempelement.endrow = -1;
            tempelement.endcolumn = -1;
            elements[y][x] = tempelement;
            break;
        case 4:
            tempelement.type = TASK;
            tempelement.startrow = -1;
            tempelement.startcolumn = -1;
            tempelement.endrow = -1;
            tempelement.endcolumn = -1;
            elements[y][x] = tempelement;
            break;
        case 5:
            break;
        case 6:
            tempelement.type = ELEMENT_NONE;
            tempelement.startrow = -1;
            tempelement.startcolumn = -1;
            tempelement.endrow = -1;
            tempelement.endcolumn = -1;
            elements[y][x] = tempelement;
            break;
        default:
            tempelement.type = START_EVENT;
            tempelement.startrow = -1;
            tempelement.startcolumn = -1;
            tempelement.endrow = -1;
            tempelement.endcolumn = -1;
            elements[y][x] = tempelement;
    }
    return;
}

void renderCanvas()
{
    for (int j = canvasOffsetY; j < MIN(12 + canvasOffsetY, ROWS); j++) {
        for (int i = canvasOffsetX; i < MIN(16 + canvasOffsetX, COLUMNS); i++) {
            int screenX = (i - canvasOffsetX) * SIZE_X;
            int screenY = (j - canvasOffsetY) * SIZE_Y;
            
            if (elements[j][i].type == START_EVENT) {
                drawCircle(screenX, screenY, GREENFANCY);
            }
            else if (elements[j][i].type == END_EVENT) {
                drawCircle(screenX, screenY, REDFANCY);
            }
            else if (elements[j][i].type == MID_EVENT) {
                drawCircle(screenX, screenY, BLACKFANCY); 
            }
            else if (elements[j][i].type == GATEWAY)
            {
                drawDiamond(screenX, screenY, WHITEFANCY);
            }
            else if (elements[j][i].type == TASK)
            {
                drawRectangle(screenX, screenY, WHITEFANCY);
            }
            // Add other element types here
        }
    }
    for (int i = 0; i < totalflows; i++)
    {
        BPMNElement currentArrow = flows[i];   
        drawArrow(currentArrow, BLUEFANCY);
    }
    return;
}

// Simplified arrowhead drawing






int main(int argc, char *argv[])
{
    touchPosition touch;

    for (int i = 0; i < 20; i++)
    {
    for (int j = 0; j < 50; j++)
    {
        BPMNElement tempelement;
        tempelement.type = ELEMENT_NONE;
        tempelement.startrow = -1;
        tempelement.startcolumn = -1;
        tempelement.endrow = -1;
        tempelement.endcolumn = -1;
        elements[i][j] = tempelement;
        break;
    }
    }

    videoSetMode(MODE_5_2D); //Initiallizing Video for Top Screen
	videoSetModeSub(MODE_5_2D); //Initiallizing Video for Bottom Screen

    PrintConsole topScreen;
	//PrintConsole bottomScreen;

    vramSetBankA(VRAM_A_MAIN_BG); //Selecting slot A of VRAM to be allocated as a background layer
    vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG); //Selecting slot C of VRAM to be allocated as a background layer
    vramSetBankD(VRAM_D_LCD); 

    consoleInit(&topScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	//consoleInit(NULL, 3, BgType_Bmp8, BgSize_T_256x256, 31, 0, false, true);
    oamInit(&oamMain, SpriteMapping_1D_128, false);
    setBackdropColorSub(GRAY);
    //turning on consoles

    //consoleSelect(&topScreen);
    //setBackdropColor(GRAY);
    //setBackdropColorSub(GRAY);

    int bg3 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);
    int bgs3 = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);
    dmaCopy(topscreenbackgroundBitmap, bgGetGfxPtr(bg3), topscreenbackgroundBitmapLen);
    dmaCopy(topscreenbackgroundPal, BG_PALETTE, topscreenbackgroundPalLen); //Adding Icons and Text because rendering is for bitches

    u16* cursor2 = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
    for(int i = 0; i < 32 * 32 / 2; i++)
	{  
        cursor2[i] = 1 | (1 << 8);
    }
    SPRITE_PALETTE[1] = RGB15(225, 225, 50);

    

    //u16* bottomBuffer = BG_GFX_SUB;

    while (pmMainLoop())
    {
        swiWaitForVBlank();
		scanKeys();
        touchRead(&touch);
		int keys = keysDown();

		if(keys & KEY_START) break;
        else if (keys & KEY_L)
        {
            if (topScreenCursor == 0)
            {
                topScreenCursor = 6;
            }
            else
            {
                topScreenCursor--;
            }
            drawCursor(cursor2);
        }
        else if (keys & KEY_R)
        {
            if (topScreenCursor == 6)
            {
                topScreenCursor = 0;
            }
            else
            {
                topScreenCursor++;
            }
            drawCursor(cursor2);
        }
        else if (keys & KEY_DOWN)
        {
            if (canvasOffsetY < 5)
            {
                canvasOffsetY++;
                resetScreen();
                renderCanvas();
            }
        }
        else if (keys & KEY_RIGHT)
        {
            if (canvasOffsetX < 5)
            {
                canvasOffsetX++;
                resetScreen();
                renderCanvas();
            }
        }
        else if (keys & KEY_UP)
        {
            if (canvasOffsetY > 0)
            {
                canvasOffsetY--;
                resetScreen();
                renderCanvas();
            }
        }
        else if (keys & KEY_LEFT)
        {
            if (canvasOffsetX > 0)
            {
                canvasOffsetX--;
                resetScreen();
                renderCanvas();
            }
        }
        else if (keys & KEY_TOUCH)
        {
            int newx = touch.px / SIZE_X;
            int newy = touch.py / SIZE_Y;
            //highlightGrid(touch.px, touch.py);
            if (currentTool != 5)
            {
                arrowdrawn = false;
                addFigure(newx + canvasOffsetX, newy + canvasOffsetY);
            }
            else
            {
                if (arrowdrawn)
                {
                    BPMNElement tempelement;
                    tempelement.type = FLOW;
                    tempelement.startrow = arrowrow;
                    tempelement.startcolumn = arrowcolumn;
                    tempelement.endrow = newy + canvasOffsetY;
                    tempelement.endcolumn = newx + canvasOffsetX;
                    flows[totalflows] = tempelement;
                    totalflows++;
                    arrowdrawn = false;
                }
                else
                {
                    arrowrow = newy + canvasOffsetY;
                    arrowcolumn = newx + canvasOffsetX;
                    arrowdrawn = true;
                }

            }
            resetScreen();
            renderCanvas();
            //drawCircle(newx * 16, newy * 16, GREENFANCY);
        }      
    }
    return 0;
    }
