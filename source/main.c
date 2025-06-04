#include <nds.h>
#include <stdio.h>
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
	vramSetBankC(VRAM_C_SUB_BG); //Selecting slot C of VRAM to be allocated as a background layer
    vramSetBankD(VRAM_D_SUB_SPRITE); //Selecting Slot D to draw Sprites (Nodes)

    consoleInit(&topScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
    //turning on consoles

    consoleSelect(&topScreen);
    iprintf("\tI'm Surprised this shit works\n");
    
    setBackdropColor(GRAY);
    setBackdropColorSub(GRAY); //testing

    int bg3 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);
    dmaCopy(topscreenbackgroundBitmap, bgGetGfxPtr(bg3), topscreenbackgroundBitmapLen);
    dmaCopy(topscreenbackgroundPal, BG_PALETTE, topscreenbackgroundPalLen);
    
    while (pmMainLoop())
    {
        touchRead(&touch);

        iprintf("\x1b[10;0HTouch x = %04i, %04i\n", touch.rawx, touch.px);
		iprintf("Touch y = %04i, %04i\n", touch.rawy, touch.py);

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
        }
    }
    return 0;
}
