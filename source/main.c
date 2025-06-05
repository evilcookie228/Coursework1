#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "topscreenbackground.h"

typedef enum {
    ELEMENT_NONE,
    START_EVENT,
    END_EVENT,
    MID_EVENT,
    TASK,
    GATEWAY,
    FLOW
} BPMNElementType;

// Structure for BPMN elements
typedef struct {
    BPMNElementType type;
    int x, y;
    int width, height;
    int id;
    int connectedTo[10];
    int connectionCount;
} BPMNElement;

#define MAX_ELEMENTS 100
#define GREEN RGB15(0, 31, 0)
#define RED RGB15(31, 0, 0)
#define WHITE RGB15(31, 31, 31)
#define BLACK RGB15(0, 0, 0)
#define GRAY RGB15(15, 15, 15)
#define BLUE RGB15(0, 0, 31)
BPMNElement elements[MAX_ELEMENTS];
BPMNElementType currentTool = TASK;
int canvasOffsetX = 0;
int canvasOffsetY = 0;
int drawingFlow = 0;
int flowStartElement = -1;
int topScreenCursor = 0;



void drawCursor(u16* cursor1)
{
    switch (topScreenCursor)
    {
        case 0:
        oamSet(&oamMain, 3, 45, 60, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
        case 1:
        oamSet(&oamMain, 3, 115, 60, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
        case 2:
        oamSet(&oamMain, 3, 190, 60, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
        case 3:
        oamSet(&oamMain, 3, 45, 145, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
        case 4:
        oamSet(&oamMain, 3, 116, 145, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
			cursor1, -1, false, false, false, false, false);
        case 5:
        oamSet(&oamMain, 3, 190, 145, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color,
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

int main(int argc, char *argv[])
{
    touchPosition touch;

    videoSetMode(MODE_5_2D); //Initiallizing Video for Top Screen
	videoSetModeSub(MODE_5_2D); //Initiallizing Video for Bottom Screen

    PrintConsole topScreen;
	PrintConsole bottomScreen;

    vramSetBankA(VRAM_A_MAIN_BG); //Selecting slot A of VRAM to be allocated as a background layer
    vramSetBankA(VRAM_A_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG); //Selecting slot C of VRAM to be allocated as a background layer
    vramSetBankD(VRAM_D_SUB_SPRITE); //Selecting Slot D to draw Sprites (Nodes)

    consoleInit(&topScreen, 2,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 2,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
    oamInit(&oamMain, SpriteMapping_1D_128, false);
    //turning on consoles

    consoleSelect(&topScreen);
    //setBackdropColor(GRAY);
    setBackdropColorSub(GRAY);

    int bg0 = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 0,0);
    dmaCopy(topscreenbackgroundBitmap, bgGetGfxPtr(bg0), topscreenbackgroundBitmapLen);
    dmaCopy(topscreenbackgroundPal, BG_PALETTE, topscreenbackgroundPalLen); //Adding Icons and Text because rendering is for bitches
    
    u16* cursor2 = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
    for(int i = 0; i < 32 * 32 / 2; i++)
	{  
        cursor2[i] = 1 | (1 << 8);
    }
    SPRITE_PALETTE[0] = RGB15(225, 225, 50);

    while (pmMainLoop())
    {
        touchRead(&touch);

		swiWaitForVBlank();
		scanKeys();

		int keys = keysDown();

		if(keys & KEY_START) break;
        else if (keys & KEY_LEFT)
        {
            if (topScreenCursor == 0)
            {
                topScreenCursor = 5;
            }
            else
            {
                topScreenCursor--;
            }
            drawCursor(cursor2);
        }
        else if (keys & KEY_RIGHT)
        {
            if (topScreenCursor == 5)
            {
                topScreenCursor = 0;
            }
            else
            {
                topScreenCursor++;
            }
            drawCursor(cursor2);
        }
    }
    return 0;
}
