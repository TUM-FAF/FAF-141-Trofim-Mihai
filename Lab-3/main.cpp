#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include "resource.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");
HINSTANCE hInst;

void updateColorControls(HDC, COLORREF, int, int);
int getFromInput(HWND);
POINT getCurrentPointPosition(int, int, RECT, int);
void fillWithGradient(HDC hdc, HBRUSH hBrush, RECT tempRect, RECT gradientRect, int colorNr);

COLORREF fillColor = RGB(255, 255, 255);
COLORREF borderColor = RGB(0, 0, 0);
COLORREF colorSelect(HWND hwnd, COLORREF color)
{
    COLORREF g_rgbCustom[16] = {0};
    CHOOSECOLOR cc = {sizeof(CHOOSECOLOR)};

    cc.Flags = CC_RGBINIT | CC_SOLIDCOLOR;
    cc.hwndOwner = hwnd;
    cc.rgbResult = color;
    cc.lpCustColors = g_rgbCustom;

    if(ChooseColor(&cc))
    {
        color = cc.rgbResult;
    }
    //InvalidateRect(hwnd, NULL,FALSE);
    return color;
}

// Redirecting child messages to parent window
WNDPROC GroupBoxProc;
LONG CALLBACK GroupRelay(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if(msg == WM_COMMAND || msg == WM_NOTIFY){
        return SendMessage(GetParent(hwnd), msg, wParam, lParam);
    }

    return CallWindowProc(GroupBoxProc, hwnd, msg, wParam, lParam);
}


int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) CreateSolidBrush(RGB(50,150,250));

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("PW lab#3"),       /* Title Text */
           WS_OVERLAPPEDWINDOW| WS_BORDER | WS_SYSMENU, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           900,                 /* The programs width */
           550,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);
    UpdateWindow(hwnd); //make sure the window is updated correctly

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndToolsGroup, hwndPencilTool, hwndLineTool, hwndBezierTool, hwndRectangleTool, hwndEllipseTool, hwndEraserTool, hwndColorGroup,
                hwndWidthGroup, hwndFillCheck, hwndBorderWidth, hwndEraserWidth, hwndClearButton;

    RECT rect ;
    PAINTSTRUCT ps;
    HDC hdc = GetDC(hwnd);

    int screenW, screenH;
    int xMouse, yMouse;

    // Color preview rectangles
    int xFillPreview = 115;
    int yFillPreview = 330;
    int xStrokePreview = 115;
    int yStrokePreview = 355;

    HDC hdcMem;
    BITMAP bitmap;
    HBITMAP hbmpBitmapImage = NULL;
    hbmpBitmapImage = (HBITMAP)LoadImage(hInst, "bitmap.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    GetObject(hbmpBitmapImage, sizeof(bitmap), &bitmap);

    static RECT drawingArea = {170, 17, 780, 475};
    static RECT fillColorRect = {xFillPreview, yFillPreview, xFillPreview + 25, yFillPreview + 20};
    static RECT borderColorRect = {xStrokePreview, yStrokePreview, xStrokePreview + 25, yStrokePreview + 20};

    static RECT redGradientRect = {170, 480, 370, 505};
    static RECT greenGradientRect = {375, 480, 575, 505};
    static RECT blueGradientRect = {580, 480, 780, 505};

    static RECT tempRect;

    HBRUSH hBrush;
    static POINT pointPen;
    POINT point;
    HPEN linePen;
    int width;

    static BOOL lineStarted, rectangleStarted, ellipseStarted;
    static RECT rectangle, ellipse;
    static int bezierStage = 0;
    static POINT line;
    static POINT bezierPoints[4];

    HPEN borderPen;
    HBRUSH fillBrush;

    switch (message)                  /* handle the messages */
    {

        case WM_CREATE:
            screenW = GetSystemMetrics(SM_CXSCREEN);
            screenH = GetSystemMetrics(SM_CYSCREEN);
            GetWindowRect(hwnd, &rect);
            SetWindowPos(hwnd, 0, (screenW - rect.right)/2, (screenH - rect.bottom)/2, 0, 0, SWP_NOZORDER|SWP_NOSIZE);


            hwndToolsGroup = CreateWindowEx(
                0,
                "Button",
                "Tools",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                15, 130,
                140, 140,
                hwnd,
                (HMENU)IDB_TOOLS_GROUP,
                hInst,
                NULL);

            hwndColorGroup = CreateWindowEx(
                0,
                "Button",
                "Properties",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                15, 290,
                140, 90,
                hwnd,
                (HMENU)IDB_COLOR_GROUP,
                hInst,
                NULL);

             hwndWidthGroup = CreateWindowEx(
                0,
                "Button",
                "Width",
                WS_VISIBLE |WS_CHILD | BS_GROUPBOX,
                15, 400,
                140, 65,
                hwnd,
                (HMENU)IDB_WIDTH_GROUP,
                hInst,
                NULL);

             hwndPencilTool = CreateWindowEx(
                0,
                "Button",
                "Pencil",
                WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                10, 15,
                120, 20,
                hwndToolsGroup,
                (HMENU)IDB_PENCIL_TOOL,
                hInst,
                NULL);
            Button_SetCheck(hwndPencilTool, BST_CHECKED);

              hwndLineTool = CreateWindowEx(
                0,
                "Button",
                "Line",
                WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                10, 35,
                120, 20,
                hwndToolsGroup,
                (HMENU)IDB_LINE_TOOL,
                hInst,
                NULL);

            hwndBezierTool = CreateWindowEx(
                0,
                "Button",
                "Bezier",
                WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                10, 55,
                120, 20,
                hwndToolsGroup,
                (HMENU)IDB_ERASER_TOOL,
                hInst,
                NULL);

            hwndRectangleTool = CreateWindowEx(
                0,
                "Button",
                "Rectangle",
                WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                10, 75,
                120, 20,
                hwndToolsGroup,
                (HMENU)IDB_RECTANGLE_TOOL,
                hInst,
                NULL);

            hwndEllipseTool = CreateWindowEx(
                0,
                "Button",
                "Ellipse",
                WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                10, 95,
                120, 20,
                hwndToolsGroup,
                (HMENU)IDB_ELLIPSE_TOOL,
                hInst,
                NULL);

            hwndEraserTool = CreateWindowEx(
                0,
                "Button",
                "Eraser",
                WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                10, 115,
                120, 20,
                hwndToolsGroup,
                (HMENU)IDB_ERASER_TOOL,
                hInst,
                NULL);

            hwndFillCheck = CreateWindowEx(
                0,
                "Button",
                "Fill object",
                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                10, 15,
                120, 20,
                hwndColorGroup,
                (HMENU)IDB_FILL_CHECK,
                hInst,
                NULL);

            CreateWindowEx(
                0,
                "Static",
                "Fill color",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                10, 40,
                90, 20,
                hwndColorGroup,
                (HMENU)0,
                hInst,
                NULL);

            CreateWindowEx(
                0,
                "Static",
                "Border color",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                10, 65,
                90, 20,
                hwndColorGroup,
                (HMENU)0,
                hInst,
                NULL);

            CreateWindowEx(
                0,
                "Static",
                "Border width",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                10, 15,
                100, 20,
                hwndWidthGroup,
                (HMENU)0,
                hInst,
                NULL);

            hwndBorderWidth = CreateWindowEx(
                0,
                "Edit",
                "1",
                WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER,
                100, 15,
                25, 20,
                hwndWidthGroup,
                (HMENU)0,
                hInst,
                NULL);

            CreateWindowEx(
                0,
                "Static",
                "Eraser width",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                10, 40,
                100, 20,
                hwndWidthGroup,
                (HMENU)0,
                hInst,
                NULL);

            hwndEraserWidth = CreateWindowEx(
                0,
                "Edit",
                "1",
                WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER,
                100, 40,
                25, 20,
                hwndWidthGroup,
                (HMENU)0,
                hInst,
                NULL);

            hwndClearButton = CreateWindowEx(
                NULL,
                "Button",
                "Clear",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                15, 480,
                140, 25,
                hwnd,
                (HMENU)IDB_CLEAR_BUTTON,
                GetModuleHandle(NULL),
                NULL);


            RegisterHotKey(hwnd, HK_DRAW_ELLIPSE, MOD_CONTROL, 0x45); //CTRL+E
            RegisterHotKey(hwnd, HK_BLUE_COLOR, MOD_CONTROL, 0x43); // CTRL+C


            GroupBoxProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwndToolsGroup, GWLP_WNDPROC));
            SetWindowLongPtr(hwndToolsGroup, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(GroupRelay));

            GroupBoxProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwndColorGroup, GWLP_WNDPROC));
            SetWindowLongPtr(hwndColorGroup, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(GroupRelay));

            GroupBoxProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwndWidthGroup, GWLP_WNDPROC));
            SetWindowLongPtr(hwndWidthGroup, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(GroupRelay));
            break;


        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);

            updateColorControls(hdc, fillColor, xFillPreview, yFillPreview);

            updateColorControls(hdc, borderColor, xStrokePreview, yStrokePreview);

            hdcMem = CreateCompatibleDC(hdc);
            SelectObject(hdcMem, hbmpBitmapImage);
            BitBlt(hdc, 15, 15, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
            DeleteDC(hdcMem);

            SelectObject(hdc, CreatePen(PS_SOLID, 1, RGB(0,0,0)));
            SelectObject(hdc, (HBRUSH)GetStockObject(WHITE_BRUSH));
            Rectangle(hdc, drawingArea.left, drawingArea.top, drawingArea.right, drawingArea.bottom);

            tempRect.top = redGradientRect.top;
            tempRect.bottom = redGradientRect.bottom;
            fillWithGradient(hdc, hBrush, tempRect, redGradientRect, 0);

            tempRect.top = greenGradientRect.top;
            tempRect.bottom = greenGradientRect.bottom;
            fillWithGradient(hdc, hBrush, tempRect, greenGradientRect, 1);

            tempRect.top = blueGradientRect.top;
            tempRect.bottom = blueGradientRect.bottom;
            fillWithGradient(hdc, hBrush, tempRect, blueGradientRect, 2);

            EndPaint(hwnd, &ps);
            break;

         case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDB_CLEAR_BUTTON:
                    Button_SetCheck(hwndPencilTool, BST_UNCHECKED);
                    Button_SetCheck(hwndLineTool, BST_UNCHECKED);
                    Button_SetCheck(hwndBezierTool, BST_UNCHECKED);
                    Button_SetCheck(hwndRectangleTool, BST_UNCHECKED);
                    Button_SetCheck(hwndEllipseTool, BST_UNCHECKED);
                    Button_SetCheck(hwndEraserTool, BST_UNCHECKED);

                    InvalidateRect(hwnd, &drawingArea, FALSE);
                    InvalidateRect(hwnd, &drawingArea, TRUE);
                    break;
                default:
                    DefWindowProc(hwnd, WM_COMMAND, wParam, lParam);
                    break;
            }
            break;

         case WM_GETMINMAXINFO:
         {
             LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
             mmi->ptMinTrackSize.x = 800;
             mmi->ptMinTrackSize.y = 550;
             mmi->ptMaxTrackSize.x = 850;
             mmi->ptMaxTrackSize.y = 600;
             break;
         }
         case WM_LBUTTONDOWN:
            xMouse = GET_X_LPARAM(lParam);
            yMouse = GET_Y_LPARAM(lParam);

            //when pressing to choose color
            if(xMouse >= fillColorRect.left && xMouse <= fillColorRect.right)
            {
                if(yMouse >= fillColorRect.top && yMouse <= fillColorRect.bottom)
                {
                    fillColor = colorSelect(hwnd, fillColor);
                    updateColorControls(hdc, fillColor, xFillPreview, yFillPreview);
                }
                else if(yMouse >= borderColorRect.top && yMouse <= borderColorRect.bottom)
                {
                    borderColor = colorSelect(hwnd, borderColor);
                    updateColorControls(hdc, borderColor, xStrokePreview, yStrokePreview);
                }
                return 0;
            }

            if( (xMouse > drawingArea.left) && (xMouse < drawingArea.right) &&
                (yMouse > drawingArea.top) && (yMouse < drawingArea.bottom) )
            {
                width = getFromInput(hwndBorderWidth);
                point = getCurrentPointPosition(xMouse, yMouse, drawingArea, width);
                xMouse = point.x;
                yMouse = point.y;

                if((wParam == MK_LBUTTON) && (Button_GetCheck(hwndPencilTool) == BST_CHECKED))
                {
                    pointPen.x = xMouse;
                    pointPen.y = yMouse;
                }

                if((wParam == MK_LBUTTON) && (Button_GetCheck(hwndLineTool) == BST_CHECKED))
                {
                    line.x = xMouse;
                    line.y = yMouse;
                    lineStarted = true;
                }

                if((wParam == MK_LBUTTON) && (Button_GetCheck(hwndRectangleTool) == BST_CHECKED))
                {
                    rectangle.left = xMouse;
                    rectangle.top = yMouse;
                    rectangleStarted = true;
                }

                if((wParam == MK_LBUTTON) && (Button_GetCheck(hwndEllipseTool) == BST_CHECKED))
                {
                    ellipse.left = xMouse;
                    ellipse.top = yMouse;
                    ellipseStarted = true;
                }

                if((wParam == MK_LBUTTON) && (Button_GetCheck(hwndBezierTool) == BST_CHECKED))
                {
                    if(bezierStage == 0)
                    {
                        bezierPoints[0] = point;
                        bezierStage = 1;
                    }
                    else
                    {
                        bezierPoints[2] = point;
                        bezierStage = 3;
                    }
                }
            }
            break;

        case WM_LBUTTONUP:
            xMouse = GET_X_LPARAM(lParam);
            yMouse = GET_Y_LPARAM(lParam);
            width = getFromInput(hwndBorderWidth);
            point = getCurrentPointPosition(xMouse, yMouse, drawingArea, width);
            xMouse = point.x;
            yMouse = point.y;

            borderPen = CreatePen(PS_SOLID, width, borderColor);
            if(Button_GetCheck(hwndFillCheck) == BST_CHECKED)
                fillBrush = CreateSolidBrush(fillColor);
            else
                fillBrush = (HBRUSH)GetStockObject(NULL_BRUSH);

            if(lineStarted)
            {
                SelectObject(hdc, borderPen);
                MoveToEx(hdc, xMouse, yMouse, NULL);
                LineTo(hdc, line.x, line.y);
                DeleteObject(borderPen);
                lineStarted = false;
            }

            if(rectangleStarted)
            {
                SelectObject(hdc, borderPen);
                SelectObject(hdc, fillBrush);
                Rectangle(hdc, rectangle.left, rectangle.top, xMouse, yMouse);

                DeleteObject(borderPen);
                DeleteObject(fillBrush);

                rectangleStarted = false;
            }

            if(ellipseStarted)
            {
                SelectObject(hdc, borderPen);
                SelectObject(hdc, fillBrush);

                Ellipse(hdc, ellipse.left, ellipse.top, xMouse, yMouse);
                DeleteObject(borderPen);
                DeleteObject(fillBrush);

                ellipseStarted = false;
            }

            if(bezierStage == 1)
            {
                bezierPoints[1] = point;
                bezierStage = 2;
            }

            if(bezierStage == 3)
            {
                bezierPoints[3] = point;
                bezierStage = 0;
                SelectObject(hdc, borderPen);
                PolyBezier(hdc, bezierPoints, 4);
                DeleteObject(borderPen);
            }
            break;

        case WM_MOUSEMOVE:
            xMouse = GET_X_LPARAM(lParam);
            yMouse = GET_Y_LPARAM(lParam);
            if( (xMouse > drawingArea.left) && (xMouse < drawingArea.right) &&
                (yMouse > drawingArea.top) && (yMouse < drawingArea.bottom) )
            {
                if((wParam == MK_LBUTTON) && (Button_GetCheck(hwndPencilTool) == BST_CHECKED))
                {
                    width = getFromInput(hwndBorderWidth);
                    point = getCurrentPointPosition(xMouse, yMouse, drawingArea, width);
                    xMouse = point.x;
                    yMouse = point.y;
                    linePen = CreatePen(PS_SOLID, width, borderColor);
                    SelectObject(hdc, linePen);
                    MoveToEx(hdc, xMouse, yMouse, NULL);
                    LineTo(hdc, pointPen.x, pointPen.y);
                    DeleteObject(linePen);
                    pointPen.x = xMouse;
                    pointPen.y = yMouse;
                }

                if((wParam == MK_LBUTTON) && (Button_GetCheck(hwndEraserTool) == BST_CHECKED))
                {
                    width = getFromInput(hwndEraserWidth);
                    point = getCurrentPointPosition(xMouse, yMouse, drawingArea, width);
                    xMouse = point.x;
                    yMouse = point.y;
                    rect.left = point.x - (width / 2);
                    rect.right = point.x + (width / 2);
                    rect.top = point.y - (width / 2);
                    rect.bottom = point.y + (width / 2);
                    InvalidateRect(hwnd, &rect, FALSE);
                    SendMessage(hwnd, WM_PAINT, 0, 0);
                }
            }
            break;

        case WM_CTLCOLORSTATIC:
            if((GetDlgItem(hwnd, IDB_TOOLS_GROUP) == (HWND)lParam)  || (GetDlgItem(hwnd, IDB_COLOR_GROUP) == (HWND)lParam) ||
               (GetDlgItem(hwnd, IDB_WIDTH_GROUP) == (HWND)lParam));
            {
                HDC hDC = (HDC)wParam;
                SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
                SetTextColor(hDC, RGB(255,69,0));
                SetBkMode(hDC, TRANSPARENT);
                return (INT_PTR)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
            }
            break;

        case WM_HOTKEY:
        {
            switch(wParam)
            {
                case HK_DRAW_ELLIPSE:

                    Button_SetCheck(hwndPencilTool, BST_UNCHECKED);
                    Button_SetCheck(hwndLineTool, BST_UNCHECKED);
                    Button_SetCheck(hwndBezierTool, BST_UNCHECKED);
                    Button_SetCheck(hwndRectangleTool, BST_UNCHECKED);
                    Button_SetCheck(hwndEraserTool, BST_UNCHECKED);
                    Button_SetCheck(hwndEllipseTool, BST_CHECKED);
                    break;

                case HK_BLUE_COLOR:
                    Button_SetCheck(hwndFillCheck, BST_CHECKED);
                    fillColor = RGB(0,0,255);
                    updateColorControls(hdc, fillColor, xFillPreview, yFillPreview);
                    break;
                default:
                    break;
            }
            break;
        }

        case WM_CLOSE:
            if(MessageBox(hwnd, "Do you want to exit?", "Lab#3", MB_YESNO) == IDYES) DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}

void updateColorControls(HDC hdc, COLORREF rgb, int xLeft, int yTop)
{
    HBRUSH hBrush = CreateSolidBrush(rgb);
    HPEN hPen = CreatePen(PS_INSIDEFRAME, 2, RGB(10, 40, 140));

    SelectObject(hdc, hBrush);
    SelectObject(hdc, hPen);

    Rectangle(hdc, xLeft, yTop, xLeft + 25, yTop + 20);

    hBrush = CreateSolidBrush(RGB(10, 40, 140));
    SelectObject(hdc, hBrush);

    Rectangle(hdc, xLeft+18, yTop+13, xLeft + 25, yTop + 20);

    DeleteObject(hPen);
    DeleteObject(hBrush);
}

int getFromInput(HWND input)
{
    int result, len;
    len = SendMessage(input, WM_GETTEXTLENGTH, 0, 0);

    char * temp = (char *)malloc(len + 1);
    SendMessage(input, WM_GETTEXT, len + 1, (LPARAM)temp);
    result = atoi(temp);
    free(temp);
    return result;
}

POINT getCurrentPointPosition(int xMouse, int yMouse, RECT limit, int width)
{
    POINT result;
    width = width / 2 + 1;

    if(xMouse + width > limit.right)
        result.x = limit.right - width;
    else if(xMouse - width < limit.left)
        result.x = limit.left + width;
    else
        result.x = xMouse;

    if(yMouse - width < limit.top)
        result.y = limit.top + width;
    else if(yMouse + width > limit.bottom)
        result.y = limit.bottom - width;
    else
        result.y = yMouse;

    return result;
}

void fillWithGradient(HDC hdc, HBRUSH hBrush, RECT tempRect, RECT gradientRect, int colorNr)
{
    for(int i = 0; i < (gradientRect.right - gradientRect.left); i++) {
        int color;
        color = i * 255 / (gradientRect.right - gradientRect.left);
        tempRect.left  = gradientRect.left  + i;
        tempRect.right = gradientRect.left + i + 1;
        if (colorNr == 0)
            hBrush = CreateSolidBrush(RGB(color, 0, 0));
        else if (colorNr == 1)
            hBrush = CreateSolidBrush(RGB(0, color, 0));
        else
            hBrush = CreateSolidBrush(RGB(0, 0, color));
        FillRect(hdc, &tempRect, hBrush);
        DeleteObject(hBrush);
    }
}
