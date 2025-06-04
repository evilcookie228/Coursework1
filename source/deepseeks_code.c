#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds/touch.h>
#include <nds/arm9/video.h>
#include <nds/arm9/trig_lut.h>

// Screen dimensions
#define CANVAS_WIDTH 512
#define CANVAS_HEIGHT 512
#define DISPLAY_WIDTH 256
#define DISPLAY_HEIGHT 192

// BPMN element types
typedef enum {
    ELEMENT_NONE,
    START_EVENT,
    END_EVENT,
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
int elementCount = 0;
int selectedElement = -1;
int drawingFlow = 0;
int flowStartElement = -1;
BPMNElementType currentTool = TASK;
int canvasOffsetX = 0;
int canvasOffsetY = 0;
PrintConsole bottomScreen;

// Function prototypes
void drawRoundedRect(int x, int y, int w, int h, int r, u16 color);
void drawCircle(int x, int y, int radius, u16 color);
void drawRect(int x, int y, int w, int h, u16 color);
void drawLine(int x1, int y1, int x2, int y2, u16 color);
void drawDiamond(int x, int y, int w, int h, u16 color);
void init();
void drawElement(BPMNElement *element);
int isPointInElement(int x, int y, BPMNElement *element);
int findElementAt(int x, int y);
void addElement(BPMNElementType type, int x, int y);
void connectElements(int fromId, int toId);
void updateInstructions();

// Simple max/min macros
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

// Draw a rounded rectangle
void drawRoundedRect(int x, int y, int w, int h, int r, u16 color) {
    // Corners
    drawCircle(x + r, y + r, r, color);
    drawCircle(x + w - r, y + r, r, color);
    drawCircle(x + r, y + h - r, r, color);
    drawCircle(x + w - r, y + h - r, r, color);
    
    // Borders
    drawRect(x + r, y, w - 2*r, h, color);
    drawRect(x, y + r, w, h - 2*r, color);
}

// Circle drawing function
void drawCircle(int x, int y, int radius, u16 color) {
    for(int angle = 0; angle < 360; angle++) {
        int px = x + (radius * cosLerp(angle * 10));
        int py = y + (radius * sinLerp(angle * 10));
        if(px >= 0 && px < DISPLAY_WIDTH && py >= 0 && py < DISPLAY_HEIGHT) {
            VRAM_A[py * DISPLAY_WIDTH + px] = color;
        }
    }
}

// Rectangle drawing function
void drawRect(int x, int y, int w, int h, u16 color) {
    // Horizontal lines
    for(int i = 0; i < w; i++) {
        if(x+i >= 0 && x+i < DISPLAY_WIDTH) {
            if(y >= 0 && y < DISPLAY_HEIGHT) VRAM_A[y * DISPLAY_WIDTH + (x+i)] = color;
            if(y+h-1 >= 0 && y+h-1 < DISPLAY_HEIGHT) VRAM_A[(y+h-1) * DISPLAY_WIDTH + (x+i)] = color;
        }
    }
    // Vertical lines
    for(int i = 0; i < h; i++) {
        if(y+i >= 0 && y+i < DISPLAY_HEIGHT) {
            if(x >= 0 && x < DISPLAY_WIDTH) VRAM_A[(y+i) * DISPLAY_WIDTH + x] = color;
            if(x+w-1 >= 0 && x+w-1 < DISPLAY_WIDTH) VRAM_A[(y+i) * DISPLAY_WIDTH + (x+w-1)] = color;
        }
    }
}

// Line drawing function
void drawLine(int x1, int y1, int x2, int y2, u16 color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while(1) {
        if(x1 >= 0 && x1 < DISPLAY_WIDTH && y1 >= 0 && y1 < DISPLAY_HEIGHT) {
            VRAM_A[y1 * DISPLAY_WIDTH + x1] = color;
        }
        
        if(x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if(e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if(e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Diamond drawing function
void drawDiamond(int x, int y, int w, int h, u16 color) {
    drawLine(x, y - h/2, x + w/2, y, color);
    drawLine(x + w/2, y, x, y + h/2, color);
    drawLine(x, y + h/2, x - w/2, y, color);
    drawLine(x - w/2, y, x, y - h/2, color);
}

// Initialize the DS
void init() {
    // Setup video mode
    videoSetMode(MODE_FB0);
    vramSetBankA(VRAM_A_LCD);
    
    // Setup consoles
    consoleDemoInit();
    bottomScreen = *consoleInit(&bottomScreen, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, false);
    
    // Enable touch screen
    touchInit();
    
    // Clear elements
    memset(elements, 0, sizeof(elements));
    elementCount = 0;
}

// Draw a BPMN element
void drawElement(BPMNElement *element) {
    int screenX = element->x - canvasOffsetX;
    int screenY = element->y - canvasOffsetY;
    
    // Only draw if visible on screen
    if(screenX + element->width < 0 || screenX > DISPLAY_WIDTH ||
       screenY + element->height < 0 || screenY > DISPLAY_HEIGHT) {
        return;
    }

    switch(element->type) {
        case START_EVENT:
            drawCircle(screenX, screenY, element->width/2, RGB15(0,0,0));
            break;
        case END_EVENT:
            drawCircle(screenX, screenY, element->width/2, RGB15(0,0,0));
            drawCircle(screenX, screenY, element->width/4, RGB15(0,0,0));
            break;
        case TASK:
            drawRoundedRect(screenX - element->width/2, screenY - element->height/2, 
                          element->width, element->height, 5, RGB15(0,0,0));
            break;
        case GATEWAY:
            drawDiamond(screenX, screenY, element->width, element->height, RGB15(0,0,0));
            break;
        case FLOW:
            break;
        default:
            break;
    }
    
    // Draw connections
    for(int i = 0; i < element->connectionCount; i++) {
        BPMNElement *target = NULL;
        for(int j = 0; j < elementCount; j++) {
            if(elements[j].id == element->connectedTo[i]) {
                target = &elements[j];
                break;
            }
        }
        
        if(target) {
            int targetX = target->x - canvasOffsetX;
            int targetY = target->y - canvasOffsetY;
            drawLine(screenX, screenY, targetX, targetY, RGB15(0,0,0));
        }
    }
}

// Check if point is in element
int isPointInElement(int x, int y, BPMNElement *element) {
    int elemX = element->x - canvasOffsetX;
    int elemY = element->y - canvasOffsetY;
    
    switch(element->type) {
        case START_EVENT:
        case END_EVENT:
            return (x - elemX)*(x - elemX) + (y - elemY)*(y - elemY) <= 
                   (element->width/2)*(element->width/2);
        case TASK:
            return x >= elemX - element->width/2 && x <= elemX + element->width/2 &&
                   y >= elemY - element->height/2 && y <= elemY + element->height/2;
        case GATEWAY:
            // Simplified collision for diamond
            return x >= elemX - element->width/2 && x <= elemX + element->width/2 &&
                   y >= elemY - element->height/2 && y <= elemY + element->height/2;
        default:
            return 0;
    }
}

// Find element at screen coordinates
int findElementAt(int x, int y) {
    // Check from top to bottom (reverse order) to select front elements first
    for(int i = elementCount-1; i >= 0; i--) {
        if(elements[i].type != ELEMENT_NONE && isPointInElement(x, y, &elements[i])) {
            return i;
        }
    }
    return -1;
}

// Add new element
void addElement(BPMNElementType type, int x, int y) {
    if(elementCount >= MAX_ELEMENTS) return;
    
    BPMNElement newElement;
    newElement.type = type;
    newElement.x = x + canvasOffsetX; // Store absolute position
    newElement.y = y + canvasOffsetY;
    newElement.id = elementCount + 1;
    
    // Set sizes
    switch(type) {
        case START_EVENT:
        case END_EVENT:
            newElement.width = 30;
            newElement.height = 30;
            break;
        case TASK:
            newElement.width = 80;
            newElement.height = 40;
            break;
        case GATEWAY:
            newElement.width = 40;
            newElement.height = 40;
            break;
        default:
            newElement.width = 30;
            newElement.height = 30;
    }
    
    newElement.connectionCount = 0;
    elements[elementCount++] = newElement;
}

// Connect two elements
void connectElements(int fromId, int toId) {
    if(fromId == toId) return;
    
    for(int i = 0; i < elementCount; i++) {
        if(elements[i].id == fromId) {
            // Check if already connected
            for(int j = 0; j < elements[i].connectionCount; j++) {
                if(elements[i].connectedTo[j] == toId) return;
            }
            
            if(elements[i].connectionCount < 10) {
                elements[i].connectedTo[elements[i].connectionCount++] = toId;
            }
            break;
        }
    }
}

// Update instructions on top screen
void updateInstructions() {
    consoleClear();
    
    iprintf("\x1b[0;0HBPMN Diagram Creator");
    iprintf("\x1b[2;0H-------------------");
    iprintf("\x1b[4;0HCurrent Tool: ");
    
    switch(currentTool) {
        case START_EVENT: iprintf("Start Event"); break;
        case END_EVENT:  iprintf("End Event"); break;
        case TASK:       iprintf("Task"); break;
        case GATEWAY:    iprintf("Gateway"); break;
        case FLOW:       iprintf("Flow"); break;
        default:         iprintf("None");
    }
    
    iprintf("\x1b[6;0HControls:");
    iprintf("\x1b[7;0HX - Start Event");
    iprintf("\x1b[8;0HY - End Event");
    iprintf("\x1b[9;0HA - Task");
    iprintf("\x1b[10;0HB - Gateway");
    iprintf("\x1b[11;0HL - Flow");
    iprintf("\x1b[12;0HR - Delete");
    iprintf("\x1b[13;0HSelect - Clear All");
    iprintf("\x1b[15;0HArrows - Scroll");
    iprintf("\x1b[17;0HTouch - Place/Select");
    iprintf("\x1b[19;0HElements: %d/%d", elementCount, MAX_ELEMENTS);
    iprintf("\x1b[21;0HOffset: %d,%d", canvasOffsetX, canvasOffsetY);
}

int main(void) {
    init();
    updateInstructions();
    
    while(1) {
        scanKeys();
        int keys = keysDown();
        int held = keysHeld();
        
        // Handle tool selection
        if(keys & KEY_X) currentTool = START_EVENT;
        if(keys & KEY_Y) currentTool = END_EVENT;
        if(keys & KEY_A) currentTool = TASK;
        if(keys & KEY_B) currentTool = GATEWAY;
        if(keys & KEY_L) currentTool = FLOW;
        
        // Handle scrolling with arrow keys
        int scrollSpeed = 5;
        if(held & KEY_LEFT)  canvasOffsetX = MAX(0, canvasOffsetX - scrollSpeed);
        if(held & KEY_RIGHT) canvasOffsetX = MIN(CANVAS_WIDTH - DISPLAY_WIDTH, canvasOffsetX + scrollSpeed);
        if(held & KEY_UP)    canvasOffsetY = MAX(0, canvasOffsetY - scrollSpeed);
        if(held & KEY_DOWN)  canvasOffsetY = MIN(CANVAS_HEIGHT - DISPLAY_HEIGHT, canvasOffsetY + scrollSpeed);
        
        // Handle delete
        if((keys & KEY_R) && selectedElement != -1) {
            elements[selectedElement].type = ELEMENT_NONE;
            selectedElement = -1;
        }
        
        // Handle clear all
        if(keys & KEY_SELECT) {
            memset(elements, 0, sizeof(elements));
            elementCount = 0;
            selectedElement = -1;
            drawingFlow = 0;
            flowStartElement = -1;
        }
        
        // Update instructions if needed
        static int lastTool = -1;
        if(lastTool != currentTool || keys) {
            updateInstructions();
            lastTool = currentTool;
        }
        
        // Handle touch screen
        touchPosition touch;
        touchRead(&touch);
        
        if(touch.px != 0 || touch.py != 0) {
            if(currentTool == FLOW) {
                int elementIdx = findElementAt(touch.px, touch.py);
                if(elementIdx != -1) {
                    if(!drawingFlow) {
                        flowStartElement = elementIdx;
                        drawingFlow = 1;
                    } else {
                        connectElements(elements[flowStartElement].id, elements[elementIdx].id);
                        drawingFlow = 0;
                        flowStartElement = -1;
                    }
                }
            } else {
                selectedElement = findElementAt(touch.px, touch.py);
                
                if(selectedElement == -1 && currentTool != FLOW) {
                    addElement(currentTool, touch.px, touch.py);
                    selectedElement = elementCount - 1;
                }
            }
        }
        
        // Clear bottom screen to white
        for(int i = 0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++) {
            VRAM_A[i] = RGB15(31,31,31);
        }
        
        // Draw all elements
        for(int i = 0; i < elementCount; i++) {
            if(elements[i].type != ELEMENT_NONE) {
                drawElement(&elements[i]);
            }
        }
        
        // Draw flow preview
        if(drawingFlow && flowStartElement != -1) {
            int startX = elements[flowStartElement].x - canvasOffsetX;
            int startY = elements[flowStartElement].y - canvasOffsetY;
            drawLine(startX, startY, touch.px, touch.py, RGB15(0,0,0));
        }
        
        // Highlight selected element
        if(selectedElement != -1 && elements[selectedElement].type != ELEMENT_NONE) {
            int x = elements[selectedElement].x - canvasOffsetX;
            int y = elements[selectedElement].y - canvasOffsetY;
            int w = elements[selectedElement].width;
            int h = elements[selectedElement].height;
            
            drawRect(x - w/2 - 3, y - h/2 - 3, w + 6, h + 6, RGB15(31,0,0));
        }
        
        swiWaitForVBlank();
    }
    
    return 0;
}