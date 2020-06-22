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

enum type{ ELLIPSE=1,RECTANGLE,CIRCLE,TEXT }shapeType;

typedef struct EllipseNode
{
    double cx, cy;
    double dx, dy;
    enum type t;
    bool selected;
}*EllipsePtr;
EllipsePtr tmpEll,selectedEll;

typedef struct RectangleNode
{
    double cx, cy;
    double dx, dy;
    enum type t;
    bool selected;
}*RectanglePtr;
RectanglePtr tmpRec,selectedRec;

typedef struct CircleNode
{
    double cx,cy;
    double r;
    enum type t;
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
void *tmpShape, *selectedShape;

int displayMode;  //0：不显示 1：新图形模式 2：拖动改变位置 3：拖动改变大小
int selectMode; //1-4:图形种类 0:未选中
double nowx, nowy, initx, inity; //当前的坐标,以及左键按下时的初始坐标
double lastx, lasty; //拖动过程中计算相对位移
double minDist; //最短距离
char typemapping[5][100] = {" ","Ellipse", "Rectangle", "Circle", "Text"};
char statusmapping[5][100] = {"New", "Modify"};

//TODO: 目前只画出了圆形，还要补充其他形状
//TODO: 做一个scene来提示各种操作
//TODO: 选中，改变大小和删除功能

void init(); //数据初始化
void store_shape(double, double);  
void draw_shape(void *shapePtr);   //画出图形
void update_scene();               //重新绘制整个window
void reset();                      //清屏
void get_nearest();                //通过与中心点的距离找到最近的形状
void shape_dist(void *shapePtr);   //找到与形状之间的最短距离
void new_mouse_event();
void keyboard_event();
void display_type();
bool equalfun();                   //用于比较两个元素是否相同
/*
void draw_circle();
void draw_eclipse();
void draw_rectangle();
void draw_text();

void (*draw[5])()= {NULL, draw_eclipse, draw_rectangle, draw_circle, draw_text};
*/

inline double len(double dx, double dy){ return sqrt(dx*dx+dy*dy); }


void Main()
{
    init();
    InitGraphics();
    display_type();
    SetPenColor("BLUE");

    registerMouseEvent(new_mouse_event);
    registerKeyboardEvent(keyboard_event);
}
void init()
{
    int i;
    for(i=1;i<=4;i++) shape[i] = NewLinkedList();
    selectMode = 0;
    shapeType = CIRCLE;
}

void new_mouse_event(int x, int y, int button, int event)
{
    nowx = ScaleXInches(x);
    nowy = ScaleYInches(y);
    double dx = nowx-lastx,dy = nowy-lasty;
    lastx = nowx, lasty = nowy;

    switch(event)
    {
        case BUTTON_DOWN:
        {
            if(button == LEFT_BUTTON)
            {
                initx = nowx, inity = nowy;
                lastx = initx, lasty = inity;
                if(!selectMode)
                {
                    store_shape(nowx,nowy);
                    displayMode = 1;
                }
                else displayMode = 2;  //拖动改变位置
            }
            if(button == RIGHT_BUTTON)
            {
                if(!selectMode)
                {
                    get_nearest();     //获取最近的那个图形 然后设置selectedshape
                    update_scene();
                }
                else displayMode = 3; //拖动改变大小
            } 
            break;
        }

        case BUTTON_UP:
        {
            if(button == LEFT_BUTTON)
            {
                if(!selectMode)            //如果非选中模式就存下来
                {
                    displayMode = 0;
                    //InsertNode(shape[shapeType],NULL,tmpShape);
                    
                    if(shapeType == CIRCLE)  InsertNode(shape[CIRCLE],NULL,tmpCir);
                    if(shapeType == ELLIPSE) InsertNode(shape[ELLIPSE],NULL,tmpEll);
                    if(shapeType == RECTANGLE) InsertNode(shape[RECTANGLE],NULL,tmpRec);
                    if(shapeType == TEXT) InsertNode(shape[TEXT],NULL,tmpTxt);
                    
                }
                else  displayMode = 0;  //如果是选中，那就直接跳过，停止改变，但是保留选中状态

            }
            if(button == RIGHT_BUTTON) displayMode = 0;
            break;
        }
            
        case MOUSEMOVE:
        {
            if(displayMode == 1)
            {
                //TODO: 添加其他图形
                switch(shapeType)
                {
                    case CIRCLE:
                    {
                        SetEraseMode(TRUE);
                        draw_shape(tmpCir);

                        SetEraseMode(FALSE);
                        tmpCir->r = len(nowx-tmpCir->cx,nowy-tmpCir->cy);
                        draw_shape(tmpCir);
                        update_scene();
                        break;
                    }

                }
            }

            if(displayMode == 2 && selectMode)
            {
                switch(shapeType)
                {
                    case CIRCLE:
                    {
                        SetEraseMode(TRUE);
                        draw_shape(selectedCir);
                        SetEraseMode(FALSE);
                        selectedCir->cx += dx;
                        selectedCir->cy += dy;
                        draw_shape(selectedCir);
                        break;
                    }
                    case ELLIPSE:
                    {
                        break;
                    }
                    case RECTANGLE:
                    {
                        break;
                    }
                    case TEXT:
                    {
                        break;
                    }
                }
                update_scene();
            }

            if(displayMode == 3 && selectMode)  //拖动改变大小
            {
                // TODO: 这种操作感觉不顺滑，能不能用相对位移来做
                switch(shapeType)
                {
                    case CIRCLE:
                    {
                        SetEraseMode(TRUE);
                        draw_shape(selectedCir);
                        SetEraseMode(FALSE);
                        selectedCir->r = len(nowx-selectedCir->cx, nowy-selectedCir->cy);
                        draw_shape(selectedCir);
                        break;
                    }
                    case ELLIPSE:
                    {
                        break;
                    }
                    case RECTANGLE:
                    {
                        break;
                    }
                    case TEXT:
                    {
                        break;
                    }
                }
                update_scene();
            }
            break;
        }
    }
    display_type();
}

void keyboard_event(int key, int event)
{
    switch(event)
    {
        case KEY_DOWN:
            switch(key)
            {
                case VK_F1:
                    shapeType = ELLIPSE;
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
                case VK_ESCAPE:
                {
                    if(selectMode == CIRCLE) selectedCir->selected = FALSE;
                    if(selectMode == ELLIPSE) selectedEll->selected = FALSE;
                    if(selectMode == TEXT) selectedTxt->selected = FALSE;
                    if(selectMode == RECTANGLE) selectedRec->selected = FALSE;
                    selectMode = 0;
                    update_scene();
                }
                case 8:
                {
                    if(!selectMode) break;
                    if(selectMode == CIRCLE) DeleteNode(shape[CIRCLE],selectedCir, equalfun);
                    if(selectMode == ELLIPSE) DeleteNode(shape[ELLIPSE],selectedEll, equalfun);
                    if(selectMode == TEXT) DeleteNode(shape[TEXT],selectedTxt, equalfun);
                    if(selectMode == RECTANGLE) DeleteNode(shape[RECTANGLE],selectedRec, equalfun);
                    selectMode = 0;
                    reset();
                    update_scene();
                    break;
                }
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void store_shape(double x, double y)
{
    switch(shapeType)
    {
        case ELLIPSE:
        {
            tmpEll = (EllipsePtr)malloc(sizeof(struct EllipseNode));
            tmpEll->cx = x;
            tmpEll->cy = y;
            tmpEll->dx = tmpEll->dy = 0;
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
        case ELLIPSE:
        {
            EllipsePtr p = (EllipsePtr)shapePtr;
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

void reset()
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
    displayMode = 0;
    display_type();
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
        case ELLIPSE:
        {
            EllipsePtr p = (EllipsePtr)shapePtr;
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
    selectedShape = (EllipsePtr)selectedEll;
    if(selectMode == ELLIPSE) selectedEll->selected = TRUE;
    if(selectMode == CIRCLE) selectedCir->selected = TRUE;
    if(selectMode == RECTANGLE) selectedRec->selected = TRUE;
    if(selectMode == TEXT) selectedTxt->selected = TRUE;
    shapeType = tmpShape;
    
}

void display_type()
{
    SetEraseMode(TRUE);
	StartFilledRegion(1);
	MovePen(0,0);
	DrawLine(0,0.5);
	DrawLine(3,0);
	DrawLine(0,-0.5);
	DrawLine(-3,0);
	EndFilledRegion();
    SetEraseMode(FALSE);

    char c[100]="Current type: ";
    double tx = GetCurrentX(), ty = GetCurrentY();
    strcat(c,typemapping[shapeType]);
    strcat(c,"/Current status: ");
    strcat(c,statusmapping[selectMode ? 1:0]);


    MovePen(0,0.1);
    DrawTextString(c);
    MovePen(tx, ty);
}

bool equalfun(void *obj1, void *obj2)
{
    return obj1 == obj2;
}