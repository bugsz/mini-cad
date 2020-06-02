#include "extgraph.h"
#include "graphics.h"
#include "linkedlist.h"

#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <olectl.h>
#include <mmsystem.h>
#include <wingdi.h>
#include <ole2.h>
#include <ocidl.h>
#include <winuser.h>

enum type{ ECLIPSE=1,RECTANGLE,CIRCLE,TEXT }shapeType;

typedef struct EclipseNode
{
    double cx, cy;
    double dx, dy;
    bool selected;
}*EclipsePtr;
EclipsePtr tmpEcl,selectedEcl;

typedef struct RectangleNode
{
    double cx, cy;
    double dx, dy;
    bool selected;
}*RectanglePtr;
RectanglePtr tmpRec,selectedRec;

typedef struct CircleNode
{
    double cx,cy;
    double r;
    bool selected;
}*CirclePtr;
CirclePtr tmpCir,selectedCir;

typedef struct TextNode
{
    double cx,cy;
    bool selected;
    char str[10000];
}*TextPtr;
TextPtr tmpTxt,selectedTxt;

//TODO: 数据结构定义补充

linkedlistADT shape[5];  //四种图形链表结构

int displayMode;  //0：不显示 1：新图形模式 2：选择模式
int selectMode; //1-4:图形种类 0:未选中
int nowx, nowy; //当前的坐标
double minDist; //最短距离

//TODO: 目前只画出了圆形，还要补充其他形状
//TODO: 做一个scene来提示各种操作
//TODO: 选中，改变大小和删除功能

void init(); //数据初始化
void store_shape(double, double);  //当
void draw_shape(void *shapePtr);   //画出图形
void update_scene();               //重新绘制整个window
void cls();                        //清屏
void get_nearest();                //通过与中心点的距离找到最近的形状
void shape_dist(void *shapePtr);   //找到与形状之间的最短距离
/*
void draw_circle();
void draw_eclipse();
void draw_rectangle();
void draw_text();

void (*draw[5])()= {NULL, draw_eclipse, draw_rectangle, draw_circle, draw_text};
*/

inline double len(double dx, double dy){ return sqrt(dx*dx+dy*dy); }

void mouseEvent(int x, int y, int button, int event)
{
    nowx = ScaleXInches(x);
    nowy = ScaleYInches(y);

    switch(event)
    {
        case BUTTON_DOWN:
        {
            if(button == LEFT_BUTTON)
            {
                if(!selectMode)
                {
                    store_shape(nowx,nowy);
                    displayMode = 1;
                }
                else
                {
                    //TODO: 拖动缩放
                }
            }
            if(button == RIGHT_BUTTON && !selectMode)
            {
                displayMode = 2;
                get_nearest();     //获取最近的那个图形 然后设置selectedshape
                update_scene();
            } 
            break;
        }

        case BUTTON_UP:
        {
            if(button == LEFT_BUTTON && !selectMode)
            {
                displayMode = 0;
                if(shapeType == CIRCLE)  InsertNode(shape[CIRCLE],NULL,tmpCir);
                if(shapeType == ECLIPSE) InsertNode(shape[ECLIPSE],NULL,tmpEcl);
                if(shapeType == RECTANGLE) InsertNode(shape[RECTANGLE],NULL,tmpRec);
                if(shapeType == TEXT) InsertNode(shape[TEXT],NULL,tmpTxt);
            }
            
            break;
        }
            
        case MOUSEMOVE:
        {
            if(displayMode == 1)
            {
                //TODO: 添加其他图形
                SetEraseMode(TRUE);
                draw_shape(tmpCir);


                SetEraseMode(FALSE);
                tmpCir->r = len(nowx-tmpCir->cx,nowy-tmpCir->cy);
                draw_shape(tmpCir);
                update_scene();
            }

            if(displayMode == 2 && selectMode)
            {
                SetEraseMode(TRUE);
                draw_shape(selectedCir);
                SetEraseMode(FALSE);
                selectedCir->r = len(nowx-selectedCir->cx,nowy-selectedCir->cy);
                draw_shape(selectedCir);
                update_scene();
            }
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
                case VK_DELETE:
                {
                    if(selectMode == CIRCLE) selectedCir->selected = FALSE;
                    if(selectMode == ECLIPSE) selectedEcl->selected = FALSE;
                    if(selectMode == TEXT) selectedTxt->selected = FALSE;
                    if(selectMode == RECTANGLE) selectedRec->selected = FALSE;
                    selectMode = 0;
                    update_scene();
                }
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
    SetPenColor("BLUE");
    //InitConsole();

    registerMouseEvent(mouseEvent);
    registerKeyboardEvent(keyboardEvent);
}
void init()
{
    int i;
    for(i=1;i<=4;i++) shape[i] = NewLinkedList();
    selectMode = 0;
    shapeType = CIRCLE;
    
}

void store_shape(double x, double y)
{
    switch(shapeType)
    {
        case ECLIPSE:
        {
            tmpEcl = (EclipsePtr)malloc(sizeof(struct EclipseNode));
            tmpEcl->cx = x;
            tmpEcl->cy = y;
            tmpEcl->dx = tmpEcl->dy = 0;
            break;
        }

        case CIRCLE:
        {
            tmpCir = (CirclePtr)malloc(sizeof(struct CircleNode));
            tmpCir->cx = x;
            tmpCir->cy = y;
            tmpCir->r = 0;
            tmpCir->selected = FALSE;
            //InsertNode(shape[CIRCLE],NULL,tmpCir);
            break;
        }

        case RECTANGLE:
        {
            tmpRec = (RectanglePtr)malloc(sizeof(struct RectangleNode));
            tmpRec->cx = x;
            tmpRec->cy = y;
            tmpRec->dx = 0;
            tmpRec->dy = 0;
            break;
        }

        case TEXT:
        {
            break; 
        }
    }
}


void draw_shape(void *shapePtr)
{
    switch(shapeType)
    {
        case ECLIPSE:
        {
            EclipsePtr p = (EclipsePtr)shapePtr;
            break;
        }
        case CIRCLE:
        {
            CirclePtr p = (CirclePtr)shapePtr;
            if(p->selected) SetPenColor("RED"); 
            MovePen(p->cx + p->r, p->cy);
            DrawArc(p->r,0,360);
            break;
        }
        case RECTANGLE:
            break;

        case TEXT:
            break;
    }
    SetPenColor("BLUE");
}

void cls()
{
    double h = GetWindowHeight(),w = GetWindowWidth();
	SetEraseMode(TRUE);
	StartFilledRegion(1);
	MovePen(0,0);
	DrawLine(0,h);
	DrawLine(w,0);
	DrawLine(0,-h);
	DrawLine(-w,0);
	EndFilledRegion();
	SetEraseMode(FALSE);
}
void update_scene()
{ 

    enum type tmpShape = shapeType;
    int i;
    for(i=1;i<=4;i++)
    {
        shapeType = i;
        SetEraseMode(FALSE);
        TraverseLinkedList(shape[i],draw_shape);
    }
    shapeType = tmpShape;
}

void shape_dist(void *shapePtr)
{
    switch(shapeType)
    {
        case ECLIPSE:
        {
            EclipsePtr p = (EclipsePtr)shapePtr;
            break;
        }

        case CIRCLE:
        {
            CirclePtr p = (CirclePtr)shapePtr;
            if(len(nowx-p->cx, nowy-p->cy) < minDist)
            {
                minDist = len(nowx-p->cx,nowy-p->cy);
                selectedCir = p;
                selectMode = CIRCLE;
            }
            break;
        }
        case RECTANGLE:
            break;

        case TEXT:
            break;

    }
}

void get_nearest()
{
    int i;
    enum type tmpShape = shapeType;
    minDist = 0x7fff;
    for(i=1;i<=4;i++)
    {
        shapeType = i;
        TraverseLinkedList(shape[i],shape_dist);
    }
    if(selectMode == ECLIPSE) selectedEcl->selected = TRUE;
    if(selectMode == CIRCLE) selectedCir->selected = TRUE;
    if(selectMode == RECTANGLE) selectedRec->selected = TRUE;
    if(selectMode == TEXT) selectedTxt->selected = TRUE;
    shapeType = tmpShape;
    
}