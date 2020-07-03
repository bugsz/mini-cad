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

enum type{ ELLIPSE=1,RECTANGLE,CIRCLE,TEXT,LINE}shapeType;
#define BLINK 1               /* TimerID */
#define BLINKms 300           /* Time interval */

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
    double current_y;
    bool selected;
    int textlen;
    int current;
    bool isInsert;
    char Text[10000];
}*TextPtr;
TextPtr tmpTxt ,selectedTxt;

typedef struct LineNode
{
    double cx, cy;
    double dx, dy;
    enum type t;
    bool selected;
}*LinePtr;
LinePtr tmpLine,selectedLine;

linkedlistADT shape[6];             /* five linked-list structure for shapes */
void *tmpShape, *selectedShape;     /* store newly created shape and selected shape */
int displayMode;                    /* 0: non-display 1: new 2: move to change position 3: move to resize */
int selectMode;                     /* 1-5: selected shape type 0: unselected */
bool isHome;                        /* check if home page is displaying */
double nowx, nowy, initx, inity, t_x, t_y; /* current position, initial position when left button is clicked, and position in text mode */
double lastx, lasty;                /* calculate relative displacement in moving */
double minDist;                     /* shortest distance */
char typemapping[6][100] = {" ","Ellipse", "Rectangle", "Circle", "Text", "Line"};
char statusmapping[6][100] = {"New", "Modify"};

void init();                       /* data initialization */
void store_shape(double, double);  /* store temporary shape into linked list */
void draw_shape(void *shapePtr);   /* draw shapes */
void update_scene();               /* redraw whole window */
void reset();                      /* clean screen */
void get_nearest();                /* find nearest shape */
void shape_dist(void *shapePtr);   /* calculate distance from current position to center of shape */
void new_mouse_event();            /* monitor mouse event */
void keyboard_event();             /* monitor keyboard event */
void display_type();               /* display current type */
bool equalfun();                   /* check if two elements are the same */
void PrintInstructions();          /* print instructions */
void cancelTimer(int id);          /* timer function in text mode */
void InitTimer(int TimerID);       /* timer function in text mode */
void MyTimer(int timerID);         /* timer function in text mode */
void startTimer(int id, int timeinterval);/* timer function in text mode */
void InitText(double x, double y, char *Text);/* text initialization in text mode */
void MyChar(char c);               /* input in text mode */

double len(double dx, double dy){ return sqrt(dx*dx+dy*dy); }


void Main()
{
    init();
    InitGraphics();
    PrintInstructions();
    display_type();
    SetPenColor("BLUE");

    registerMouseEvent(new_mouse_event);
    registerKeyboardEvent(keyboard_event);

    registerCharEvent(MyChar);
    registerTimerEvent(MyTimer);

    startTimer(BLINK, BLINKms);
}
void init()
{
    int i;
    for(i=1;i<=5;i++) shape[i] = NewLinkedList();
    selectMode = 0;
    isHome = TRUE;
    tmpTxt = NULL;
    shapeType = TEXT;
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
                else displayMode = 2;  /* move to change position */
            }
            if(button == RIGHT_BUTTON)
            {
                if(!selectMode)
                {
                    displayMode = 3;
                    get_nearest();     /* get nearest shape then set selectedshape */
                    update_scene();
                }
                else displayMode = 3; /* move to resize */
            }
            break;
        }

        case BUTTON_UP:
        {
            if(button == LEFT_BUTTON)
            {
                if (shapeType == TEXT)
                {
                    if(!selectMode)
                    {
                        t_x = ScaleXInches(x);
                        t_y = ScaleYInches(y);
                        InitTimer(BLINK);
                        MovePen(t_x, t_y);
                        displayMode = 0;
                        break;
                    }
                    else{
                        displayMode = 0;
                        tmpTxt = selectedTxt;
                    }
                }
                else if(!selectMode)            /* if it's unselected, then store it */
                {
                    displayMode = 0;

                    if(shapeType == CIRCLE)  InsertNode(shape[CIRCLE],NULL,tmpCir);
                    if(shapeType == ELLIPSE) InsertNode(shape[ELLIPSE],NULL,tmpEll);
                    if(shapeType == RECTANGLE) InsertNode(shape[RECTANGLE],NULL,tmpRec);
                    if(shapeType == LINE) InsertNode(shape[LINE],NULL,tmpLine);
                    if(shapeType == TEXT)
                    {
                        /* delete previously stored tmp to update */
                        DeleteNode(shape[TEXT], tmpTxt, equalfun);
                        InsertNode(shape[TEXT],NULL,tmpTxt);
                    }
                }
                else  displayMode = 0;  /* if selected, then skip it, but selectedMode remains unchanged */

            }
            if(button == RIGHT_BUTTON) displayMode = 0;
            break;
        }

        case MOUSEMOVE:
        {
            if(displayMode == 1 && !selectMode)
            {
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
                    case ELLIPSE:
                    {
                        SetEraseMode(TRUE);
                        draw_shape(tmpEll);
                        SetEraseMode(FALSE);
                        tmpEll->dx = fabs(tmpEll->cx-nowx);
                        tmpEll->dy = fabs(tmpEll->cy-nowy);
                        draw_shape(tmpEll);
                        update_scene();
                        break;
                    }
                    case RECTANGLE:
                    {
                        SetEraseMode(TRUE);
                        draw_shape(tmpRec);
                        SetEraseMode(FALSE);
                        tmpRec->dx = nowx-tmpRec->cx;
                        tmpRec->dy = nowy-tmpRec->cy;
                        draw_shape(tmpRec);
                        update_scene();
                        break;
                    }
                    case LINE:
                    {
                        SetEraseMode(TRUE);
                        draw_shape(tmpLine);
                        SetEraseMode(FALSE);
                        tmpLine->dx = nowx-tmpLine->cx;
                        tmpLine->dy = nowy-tmpLine->cy;
                        draw_shape(tmpLine);
                        update_scene();
                        break;
                    }
                }
            }

            if(displayMode == 2 && selectMode)
            {
                switch(selectMode)
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
                        SetEraseMode(TRUE);
                        draw_shape(selectedEll);
                        SetEraseMode(FALSE);
                        selectedEll->cx += dx;
                        selectedEll->cy += dy;
                        draw_shape(selectedEll);
                        break;
                    }
                    case RECTANGLE:
                    {
                        SetEraseMode(TRUE);
                        draw_shape(selectedRec);
                        SetEraseMode(FALSE);
                        selectedRec->cx += dx;
                        selectedRec->cy += dy;
                        draw_shape(selectedRec);
                        break;
                    }
                    case LINE:
                    {
                        SetEraseMode(TRUE);
                        draw_shape(selectedLine);
                        SetEraseMode(FALSE);
                        selectedLine->cx += dx;
                        selectedLine->cy += dy;
                        draw_shape(selectedLine);
                        break;
                    }
                    case TEXT:
                    {
                        MovePen(selectedTxt->cx, selectedTxt->cy);
                        SetEraseMode(TRUE);
                        StartFilledRegion(1);
                        DrawLine(GetWindowWidth(), 0);
                        DrawLine(0, GetFontHeight());
                        DrawLine(-GetWindowWidth(), 0);
                        DrawLine(0, -GetFontHeight());
                        EndFilledRegion();
                        SetEraseMode(FALSE);
                        InitText(selectedTxt->cx, selectedTxt->cy, selectedTxt->Text);
                        selectedTxt->cx += dx;
                        selectedTxt->cy += dy;
                        DeleteNode(shape[TEXT], selectedTxt, equalfun);
                        InsertNode(shape[TEXT],NULL,selectedTxt);
                        break;
                    }
                }
                update_scene();
            }

            if(displayMode == 3 && selectMode)  /* Move to resize */
            {
                switch(selectMode)
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
                        SetEraseMode(TRUE);
                        draw_shape(selectedEll);
                        SetEraseMode(FALSE);
                        selectedEll->dx = fabs(selectedEll->cx-nowx);
                        selectedEll->dy = fabs(selectedEll->cy-nowy);
                        draw_shape(selectedEll);
                        break;
                    }
                    case RECTANGLE:
                    {
                        SetEraseMode(TRUE);
                        draw_shape(selectedRec);
                        SetEraseMode(FALSE);
                        selectedRec->dx = selectedRec->cx-nowx;
                        selectedRec->dy = selectedRec->cy-nowy;
                        draw_shape(selectedRec);
                        break;
                    }
                    case LINE:
                    {

                        SetEraseMode(TRUE);
                        draw_shape(selectedLine);
                        SetEraseMode(FALSE);
                        selectedLine->dx = nowx-selectedLine->cx;
                        selectedLine->dy = nowy-selectedLine->cy;
                        draw_shape(selectedLine);
                        break;
                    }
                }
                update_scene();
            }
            break;
        }
    }
}

void keyboard_event(int key, int event)
{
    if (shapeType == TEXT && tmpTxt != NULL)
    {
        /* for char converting */
        char str[2] = {0, 0};
        /* keep for InitGraphics */
        double memory_y = GetCurrentY();
        /* stop blinking for now */
        InitTimer(BLINK);

        switch (event)
        {
            case KEY_DOWN:
                switch (key)
                {
                    case VK_LEFT:
                        /* boundary judging */
                        sprintf(str, "%c", tmpTxt->Text[0]);
                        if (GetCurrentX() >= TextStringWidth(str))
                        {
                            /* choose moving distance */
                            if (tmpTxt->Text[tmpTxt->current - 1])
                            {
                                sprintf(str, "%c", tmpTxt->Text[tmpTxt->current - 1]);
                                MovePen(GetCurrentX() - TextStringWidth(str), GetCurrentY());
                            }
                            else
                            {
                                MovePen(GetCurrentX() - TextStringWidth(&tmpTxt->Text[tmpTxt->textlen - 1]), GetCurrentY());
                            }
                            /* location update */
                            tmpTxt->current--;
                        }
                        break;
                    case VK_RIGHT:
                        /* boundary judging */
                        if (GetCurrentX() <= GetWindowWidth())
                        {
                            /* moving leftward */
                            if (tmpTxt->Text[tmpTxt->current])
                            {
                                sprintf(str, "%c", tmpTxt->Text[tmpTxt->current]);
                                MovePen(GetCurrentX() + TextStringWidth(str), GetCurrentY());
                                /* location update */
                                tmpTxt->current++;
                            }
                        }
                        break;
                    case VK_DELETE:
                        /* boundary judging */
                        if (tmpTxt->Text[tmpTxt->current])
                        {
                            /* text update */
                            for (int i = tmpTxt->current; i < tmpTxt->textlen; i++)
                            {
                                tmpTxt->Text[i] = tmpTxt->Text[i + 1];
                            }
                            tmpTxt->textlen--;
                            /* redraw */
                            InitText(0, memory_y, tmpTxt->Text);
                            MovePen(GetCurrentX() - TextStringWidth(&tmpTxt->Text[tmpTxt->current]), GetCurrentY());
                        }
                        break;
                    case VK_INSERT:
                        tmpTxt->isInsert = !tmpTxt->isInsert;

                        
                    default:
                        break;
                }
            case KEY_UP:
                tmpTxt->current_y = memory_y;
                break;
        }
        /* blink again */
        startTimer(BLINK, BLINKms);
    }

    switch (event) {
        case KEY_DOWN:
            switch (key) {
                case VK_F1:
                    shapeType = ELLIPSE;
                    display_type();
                    break;
                case VK_F2:
                    shapeType = RECTANGLE;
                    display_type();
                    break;
                case VK_F3:
                    shapeType = CIRCLE;
                    display_type();
                    break;
                case VK_F4:
                    shapeType = TEXT;
                    display_type();
                    break;
                case VK_F5:
                    shapeType = LINE;
                    display_type();
                    break;
                case VK_F6:
                    reset();
                    break;
                case VK_ESCAPE: {
                    if (selectMode == CIRCLE) selectedCir->selected = FALSE;
                    if (selectMode == ELLIPSE) selectedEll->selected = FALSE;
                    if (selectMode == TEXT) selectedTxt->selected = FALSE;
                    if (selectMode == RECTANGLE) selectedRec->selected = FALSE;
                    if (selectMode == LINE) selectedLine->selected = FALSE;
                    selectMode = 0;
                    display_type();
                    update_scene();
                    break;
                }
                case VK_HOME:
                {
                    if (isHome){
                        isHome = !isHome;
                        reset();
                        update_scene();
                    }
                    else{
                        isHome = !isHome;
                        reset();
                        PrintInstructions();
                    }
                    break;
                }
                case 8: {
                    if (!selectMode) break;
                    if (selectMode == CIRCLE) DeleteNode(shape[CIRCLE], selectedCir, equalfun);
                    if (selectMode == ELLIPSE) DeleteNode(shape[ELLIPSE], selectedEll, equalfun);
                    if (selectMode == TEXT) DeleteNode(shape[TEXT], selectedTxt, equalfun);
                    if (selectMode == RECTANGLE) DeleteNode(shape[RECTANGLE], selectedRec, equalfun);
                    if (selectMode == LINE) DeleteNode(shape[LINE], selectedLine, equalfun);
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
            tmpEll->selected = FALSE;
            break;
        }

        case CIRCLE:
        {
            tmpCir = (CirclePtr)malloc(sizeof(struct CircleNode));
            tmpCir->cx = x;
            tmpCir->cy = y;
            tmpCir->r = 0;
            tmpCir->selected = FALSE;
            break;
        }

        case RECTANGLE:
        {
            tmpRec = (RectanglePtr)malloc(sizeof(struct RectangleNode));
            tmpRec->cx = x;
            tmpRec->cy = y;
            tmpRec->dx = 0;
            tmpRec->dy = 0;
            tmpRec->selected = FALSE;
            break;
        }

        case LINE:
        {
            tmpLine = (LinePtr)malloc(sizeof(struct LineNode));
            tmpLine->cx = x;
            tmpLine->cy = y;
            tmpLine->dx = 0;
            tmpLine->dy = 0;
            tmpLine->selected = FALSE;
            break;
        }

        case TEXT:
        {
            tmpTxt = (TextPtr) malloc(sizeof(struct TextNode));
            tmpTxt->cx = x;
            tmpTxt->cy = y;
            tmpTxt->current = 0;
            tmpTxt->textlen = 0;
            tmpTxt->isInsert = FALSE;
            tmpTxt->selected = FALSE;
            tmpTxt->current_y = y;
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
                if(p->selected) SetPenColor("RED");
                MovePen(p->cx + p->dx, p->cy);
                DrawEllipticalArc(p->dx, p->dy, 0, 360);
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
            {
                RectanglePtr p = (RectanglePtr)shapePtr;
                if(p->selected) SetPenColor("RED");
                MovePen(p->cx + p->dx, p->cy + p->dy);
                DrawLine(-p->dx*2,0);
                DrawLine(0,-p->dy*2);
                DrawLine(p->dx*2,0);
                DrawLine(0,p->dy*2);
                break;
            }

            case LINE:
            {
                LinePtr p = (LinePtr)shapePtr;
                if(p->selected) SetPenColor("RED");
                MovePen(p->cx, p->cy);
                DrawLine(p->dx,p->dy);
                break;
            }

            case TEXT:
            {
                TextPtr p = (TextPtr)shapePtr;
                if(p->selected) SetPenColor("RED");
                InitText(p->cx, p->cy, p->Text);
                break;
            }

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
    for(i=1;i<=5;i++)
    {
        shapeType = i;
        SetEraseMode(FALSE);
        TraverseLinkedList(shape[i],draw_shape);
    }
    shapeType = tmpShape;
    PrintInstructions();
}

void shape_dist(void *shapePtr)
{
    switch(shapeType)
    {
        case ELLIPSE:
        {
            EllipsePtr p = (EllipsePtr)shapePtr;
            if(len(nowx-p->cx, nowy-p->cy) < minDist)
            {
                minDist = len(nowx-p->cx, nowy-p->cy);
                selectedEll = p;
                selectMode = ELLIPSE;
                display_type();
            }
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
                display_type();
            }
            break;
        }
        case RECTANGLE:
        {
            RectanglePtr p = (RectanglePtr)shapePtr;
            if(len(nowx-p->cx, nowy-p->cy) < minDist)
            {
                minDist = len(nowx-p->cx,nowy-p->cy);
                selectedRec = p;
                selectMode = RECTANGLE;
                display_type();
            }
            break;
        }
        case LINE:
        {
            LinePtr p = (LinePtr)shapePtr;
            if(len(nowx-p->cx, nowy-p->cy) < minDist)
            {
                minDist = len(nowx-p->cx,nowy-p->cy);
                selectedLine = p;
                selectMode = LINE;
                display_type();
            }
            break;
        }
        case TEXT:
        {
            TextPtr p = (TextPtr)shapePtr;
            if(len(nowx-p->cx, nowy-p->cy) < minDist)
            {
                minDist = len(nowx-p->cx,nowy-p->cy);
                selectedTxt = p;
                selectMode = TEXT;
                display_type();
            }
            break;
        }
    }
}

void get_nearest()
{
    int i;
    enum type tmpShape = shapeType;
    minDist = 0x7fff;
    for(i=1;i<=5;i++)
    {
        shapeType = i;
        TraverseLinkedList(shape[i],shape_dist);
    }
    if(minDist == 0x7fff) return ;
    if(selectMode == ELLIPSE) selectedEll->selected = TRUE;
    if(selectMode == CIRCLE) selectedCir->selected = TRUE;
    if(selectMode == RECTANGLE) selectedRec->selected = TRUE;
    if(selectMode == LINE) selectedLine->selected = TRUE;
    if(selectMode == TEXT)
    {
        selectedTxt->selected = TRUE;
        tmpTxt = selectedTxt;
        InitText(tmpTxt->cx, tmpTxt->current_y, tmpTxt->Text);
    }
    shapeType = selectMode;

}

void display_type()
{
    SetEraseMode(TRUE);
	StartFilledRegion(1);
	MovePen(0,0);
	DrawLine(0,0.5);
	DrawLine(7,0);
	DrawLine(0,-0.5);
	DrawLine(-7,0);
	EndFilledRegion();
    SetEraseMode(FALSE);

    char c[100]="Current type: ";
    double tx = GetCurrentX(), ty = GetCurrentY();
    strcat(c,typemapping[shapeType]);
    strcat(c," / Current status: ");
    strcat(c,statusmapping[selectMode ? 1:0]);

    MovePen(0,0.1);
    DrawTextString(c);
    MovePen(tx, ty);
}

bool equalfun(void *obj1, void *obj2)
{
    return obj1 == obj2;
}

void InitTimer(int TimerID)
{
    if (shapeType == TEXT)
    {
        cancelTimer(TimerID);
        SetEraseMode(TRUE);
        DrawLine(0, GetFontHeight());
        MovePen(GetCurrentX(), GetCurrentY() - GetFontHeight());
        SetEraseMode(FALSE);
    }
}
/* text reprint */
void InitText(double x, double y, char *Text)
{
    if (shapeType == TEXT)
    {
        MovePen(x, y);
        SetEraseMode(TRUE);
        StartFilledRegion(1);
        DrawLine(TextStringWidth(Text)+TextStringWidth("ab"), 0);
        DrawLine(0, GetFontHeight());
        DrawLine(-TextStringWidth(Text)-TextStringWidth("ab"), 0);
        DrawLine(0, -GetFontHeight());

        EndFilledRegion();
        SetEraseMode(FALSE);
        MovePen(x, y);
        DrawTextString(Text);
    }
}
/* Blink */
void MyTimer(int timerID)
{
    if (shapeType == TEXT)
    {
        static bool isDisplay = TRUE;
        if (isDisplay)
        {
            SetEraseMode(FALSE);
        }
        else
        {
            SetEraseMode(TRUE);
        }
        DrawLine(0, GetFontHeight());
        MovePen(GetCurrentX(), GetCurrentY() - GetFontHeight());
        isDisplay = !isDisplay;
    }
}

void MyChar(char c)
{
    if (shapeType == TEXT)
    {
        /* keep for InitGraphics */
        double memory_y = GetCurrentY();
        /* stop blinking for now */
        InitTimer(BLINK);
        if (tmpTxt->selected==TRUE) SetPenColor("RED");
        else     SetPenColor("BLUE");

        switch (c)
        {
            /* Esc */
            case 27:
                break;
            case '\r':
            {
                InsertNode(shape[TEXT],NULL,tmpTxt);
                break;
            }
                /* backspace */
            case 8:
                if (tmpTxt->Text[tmpTxt->current - 1])
                {
                    /* delete */
                    for (int i = tmpTxt->current - 1; i < tmpTxt->textlen; i++)
                    {
                        tmpTxt->Text[i] = tmpTxt->Text[i + 1];
                    }
                    tmpTxt->textlen--;
                    /* redraw */
                    if (tmpTxt->selected==TRUE) SetPenColor("RED");
                    else     SetPenColor("BLUE");
                    InitText(tmpTxt->cx, memory_y, tmpTxt->Text);
                    MovePen(GetCurrentX() - TextStringWidth(&tmpTxt->Text[--tmpTxt->current]), GetCurrentY());
                }
                else
                {
                    /* only move leftward */
                    keyboard_event(VK_LEFT, KEY_DOWN);
                }
                break;

            case VK_F1:
            case VK_F2:
            case VK_F3:
            case VK_F4:
            case VK_F5:
                break;


            default:
                /* add */
                if (tmpTxt->current == tmpTxt->textlen)
                {
                    if (tmpTxt->selected==TRUE) SetPenColor("RED");
                    else     SetPenColor("BLUE");
                    tmpTxt->Text[tmpTxt->textlen++] = c;
                    tmpTxt->Text[tmpTxt->textlen] = '\0';
                    tmpTxt->current++;
                    MovePen(tmpTxt->cx, tmpTxt->cy);
                    DrawTextString(tmpTxt->Text);
                }
                    /* insert */
                else
                {
                    /* text update */
                    if (tmpTxt->isInsert)
                    {
                        tmpTxt->Text[tmpTxt->current] = c;
                    }
                    else
                    {
                        for (int i = tmpTxt->textlen; i >= tmpTxt->current; i--)
                        {
                            tmpTxt->Text[i + 1] = tmpTxt->Text[i];
                        }
                        tmpTxt->Text[++tmpTxt->textlen] = '\0';
                        tmpTxt->Text[tmpTxt->current] = c;
                    }
                    /* redraw */
                    if (tmpTxt->selected==TRUE) SetPenColor("RED");
                    else     SetPenColor("BLUE");
                    InitText(t_x, memory_y, tmpTxt->Text);
                    MovePen(GetCurrentX() - TextStringWidth(&tmpTxt->Text[++tmpTxt->current]), GetCurrentY());
                }
                break;
        }
        /* blink again */
        startTimer(BLINK, BLINKms);
        tmpTxt->current_y = memory_y;
    }
}

void PrintInstructions()
{
    double x, y;

    x = 0;
    y = GetWindowHeight() - 0.2;
    string color = GetPenColor();
    SetPenColor("Black");

    MovePen(x,y);
    DrawTextString("  ===Mini-CAD===  ");

    y -= 0.2;
    MovePen(x,y);

    DrawTextString("  INSTRUCTIONS");
    y -= 0.2;
    MovePen(x,y);
    DrawTextString("  1. Drag the left mouse button to create a new shape; ");
    y -= 0.2;
    MovePen(x,y);
    DrawTextString("      NOTE: Under TEXT mode, press ENTER to save changes; ");
    y -= 0.2;
    MovePen(x,y);
    DrawTextString("  2. Under unselected status, right-click to select an object; ");
    y -= 0.2;
    MovePen(x,y);
    DrawTextString("  3. After the selected object turned red: ");
    y -= 0.2;
    MovePen(x,y);
    DrawTextString("      Move the object: drag the left mouse button; ");

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("      Change the shape: drag the right mouse button; ");

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("  4. Delete shape: press DELETE key;");

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("  5. Deselect: press ESC key;");

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("  6. Enter/exit instructions page: press HOME key.");

    y -= 0.2;
    MovePen(x,y);

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("  Switch the shape mode: ");

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("       Ellipse: F1");

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("  Rectangle: F2");

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("         Circle: F3");

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("           Text: F4");

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("           Line: F5");

    y -= 0.2;
    MovePen(x,y);
    DrawTextString("         Reset: F6");
    SetPenColor(color);

    return;
}

