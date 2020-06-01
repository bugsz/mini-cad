#include "extgraph.h"
#include "graphics.h"
#include "linkedlist.h"

#include<stdio.h>
#include<math.h>
#include <windows.h>
#include <olectl.h>
#include <mmsystem.h>
#include <wingdi.h>
#include <ole2.h>
#include <ocidl.h>
#include <winuser.h>

enum { ECLIPSE=1,RECTANGLE,CIRCLE,TEXT }shapeType;

typedef struct EclipseNode
{
    double cx, cy;
    double dx, dy;
}*EclipsePtr;
EclipsePtr tmpEcl;

typedef struct RectangleNode
{
    double cx, cy;
    double dx, dy;
}*RectanglePtr;
RectanglePtr tmpRec;

typedef struct CircleNode
{
    double cx,cy;
    double r;
}*CirclePtr;
CirclePtr tmpCir;

typedef struct TextNode
{
    double cx, cy;
}*TextPtr;
TextPtr tmpTxt;
// TODO 
linkedlistADT shape[5];  //四种图形链表结构

struct Node1
{
    double x,y;
    int type;
    double drx,dry;
}pastStatus,nowStatus;
double mX, mY;
int displayMode;  //0: just lift up pen 1:just put down pen 2:drawing
double lastx,lasty,lastr,initx,inity;

void init();
void store_center(double, double);
void store_shape(double, double);

inline double len(double dx, double dy){ return sqrt(dx*dx+dy*dy); }
void mouseEvent(int x, int y, int button, int event)
{
    double nx = ScaleXInches(x);
    double ny = ScaleYInches(y);

    switch(event)
    {
        case BUTTON_DOWN:
            
            if(button == LEFT_BUTTON)
            {
                lastx = initx = nx;
                lasty = inity = ny;
                lastr = 0;
                displayMode = 1;
                store_center(nx,ny);
            }
            if(button == RIGHT_BUTTON) break;   //TODO
            break;

        case BUTTON_UP:
            displayMode = 0;
            store_shape(nx,ny);
            break;

        case MOUSEMOVE:
            if(displayMode == 1)
            {
                SetEraseMode(TRUE);
                MovePen(lastx,lasty);
                DrawArc(lastr,0,360);
                SetEraseMode(FALSE);
                MovePen(nx,ny);
                double nr = len(initx-nx,inity-ny);
                DrawArc(nr,0,360);
                lastx = nx;
                lasty = ny;
                lastr = nr;
                break;
            }
            
    }
    
}

void keyboardEvent(int key, int event)
{
    switch(event)
    {
        case KEY_DOWN:
            switch(key)
            {
                case VK_F1:
                    shapeType = ECLIPSE;
                    break;
                case VK_F2:
                    shapeType = RECTANGLE;
                    break;
                case VK_F3:
                    shapeType = CIRCLE;
                    break;
                case VK_F4:
                    shapeType = TEXT;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}
void Main()
{
    init();
    InitGraphics();
    
    registerMouseEvent(mouseEvent);
    registerKeyboardEvent(keyboardEvent);
}
void init()
{
    int i;
    for(i=1;i<=4;i++) shape[i] = NewLinkedList();
}
void store_center(double x, double y)
{
    switch(shapeType)
    {
        case ECLIPSE:
            tmpEcl = (EclipsePtr)malloc(sizeof(struct EclipseNode));
            tmpEcl->cx = x;
            tmpEcl->cy = y;
            break;

        case CIRCLE:
            tmpCir = (CirclePtr)malloc(sizeof(struct CircleNode));
            tmpCir->cx = x;
            tmpCir->cy = y;
            break;

        case RECTANGLE:
            tmpRec = (RectanglePtr)malloc(sizeof(struct RectangleNode));
            tmpRec->cx = x;
            tmpRec->cy = y;
            break;

        case TEXT:
            break; 
    }
}


void store_shape(double x,double y)
{
    switch(shapeType)
    {
        case ECLIPSE:
            InsertNode(shape[ECLIPSE],NULL,tmpEcl);
            break;
        case CIRCLE:
            tmpCir->r = len(tmpCir->cx-x, tmpCir->cy-y);
            InsertNode(shape[CIRCLE],NULL,tmpCir);
            break;
        case RECTANGLE:
            InsertNode(shape[RECTANGLE],NULL,tmpRec);
            break;
        case TEXT:
            InsertNode(shape[TEXT],NULL,tmpTxt);
    }
}