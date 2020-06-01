#include "extgraph.h"
#include "graphics.h"
#include<stdio.h>
#include<math.h>

struct Node
{
    double x,y;
    int type;
    double drx,dry;
}pastStatus,nowStatus;
double mX, mY;
int inDisplay;

inline double len(double dx, double dy){ return sqrt(dx*dx+dy*dy); }
void mouseEvent(int x, int y, int button, int event)
{
    switch(event)
    {
        case BUTTON_DOWN:
            if(!inDisplay)
            {
                inDisplay = 1;
                mX = ScaleXInches(x);
                mY = ScaleYInches(y);
            }
            break;
        case MOUSEMOVE:
            if(inDisplay)
            {
                SetEraseMode(TRUE);
                MovePen(pastStatus.x,pastStatus.y);
                DrawArc(pastStatus.drx,0,360);
                SetEraseMode(FALSE);
                nowStatus.x = ScaleXInches(x);
                nowStatus.y = ScaleYInches(y);
                nowStatus.drx = len(mX-nowStatus.x,mY-nowStatus.y);
                MovePen(nowStatus.x,nowStatus.y);
                DrawArc(nowStatus.drx, 0, 360);
                pastStatus = nowStatus;
            }
            break;
        case BUTTON_UP:
            if(inDisplay) inDisplay = 0;
            break;

        default:
            break;
    }
    
}

void Main()
{
    InitGraphics();
    
    registerMouseEvent(mouseEvent);
}
