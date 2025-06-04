#include <nds.h>
#include <stdio.h>

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
BPMNElement elements[MAX_ELEMENTS];
BPMNElementType currentTool = TASK;
int canvasOffsetX = 0;
int canvasOffsetY = 0;
int drawingFlow = 0;
int flowStartElement = -1;

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

int main(int argc, char *argv[])
{
    touchPosition touch;

    videoSetMode(MODE_0_2D); //Initiallizing Video for Top Screen
	videoSetModeSub(MODE_0_2D); //Initiallizing Video for Bottom Screen

    PrintConsole topScreen;
	PrintConsole bottomScreen;

    vramSetBankA(VRAM_A_MAIN_BG); //Selecting slot A of VRAM to be allocated as a background layer
	vramSetBankC(VRAM_C_SUB_BG); //Selecting slot C of VRAM to be allocated as a background layer

    consoleInit(&topScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
    //turning on consoles

    consoleSelect(&topScreen);
    iprintf("\tI'm Surprised this shit works\n");
    
    setBackdropColor(0xF800);
    setBackdropColorSub(0xFFE0); //testing
    
    while (pmMainLoop())
    {
        touchRead(&touch);

        iprintf("\x1b[10;0HTouch x = %04i, %04i\n", touch.rawx, touch.px);
		iprintf("Touch y = %04i, %04i\n", touch.rawy, touch.py);

		swiWaitForVBlank();
		scanKeys();

		int keys = keysDown();

		if(keys & KEY_START) break;
    }
    return 0;
}
