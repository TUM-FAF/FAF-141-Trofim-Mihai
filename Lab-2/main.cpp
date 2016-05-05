#include <tchar.h>
#include <windows.h>
#include "resource.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <algorithm>

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK InfoDlgProc(HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");

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
    wincl.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (hThisInstance, MAKEINTRESOURCE(IDI_ICON));
    wincl.hIconSm = LoadIcon (hThisInstance, MAKEINTRESOURCE(IDI_ICON));
    wincl.hCursor = LoadCursor (hThisInstance, MAKEINTRESOURCE(ID_CURSOR));
    wincl.lpszMenuName = MAKEINTRESOURCE(IDM_MENU);
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("WP lab#2"),       /* Title Text */
           WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, /* default window */
           300,       /* Windows decides the position */
           200,       /* where the window ends up on the screen */
           500,                 /* The programs width */
           350,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);
    UpdateWindow(hwnd);

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
    static HINSTANCE hInstance;
    static int a, b, c, textColor = 0;
    static int cxCoord, cyCoord;
    static bool backgFlag;
    static HWND listBox, buttonAdd, submitBox, scrollBarColor,
                scrollBarWidth, scrollBarHeight;
    HBRUSH hbrush;                                                          // handle to brush
    LRESULT textSize;
    char textStore[20];
    static int xPos, xMin, xMax;



    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:
        {
            listBox = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), TEXT(""),
                             WS_VISIBLE | WS_CHILD | LBS_NOTIFY,
                             20, 45, 150, 200,
                             hwnd, (HMENU) IDC_LISTBOX, GetModuleHandle(NULL), NULL );
            buttonAdd = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("Add"),
                             WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                             180, 10, 60, 25,
                             hwnd, (HMENU) IDC_ADD, GetModuleHandle(NULL), NULL);
            submitBox = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""),
                             WS_VISIBLE | WS_CHILD,
                             20, 10, 150, 25,
                             hwnd, (HMENU)IDC_SUBMIT, GetModuleHandle(NULL), NULL);
            scrollBarColor = CreateWindowEx(NULL, TEXT("SCROLLBAR"), TEXT(""),
                             WS_VISIBLE | WS_CHILD | SBS_VERT,
                             250, 10, 220, 25,
                             hwnd, (HMENU)IDC_SCROLL_COLOR, GetModuleHandle(NULL), NULL);
            SendMessage(submitBox, EM_LIMITTEXT, WPARAM(10), NULL);
            xPos = 0;
            xMin = 0;
            xMax = 255;
            SetScrollRange(scrollBarColor, SB_CTL, xMin, xMax, FALSE);
            SetScrollPos(scrollBarColor, SB_CTL, xPos, TRUE);

            RegisterHotKey(hwnd, HK_EXIT, MOD_CONTROL, 0x45);
            RegisterHotKey(hwnd, HK_INFO, MOD_CONTROL, 0x49);
            hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
            break;
        }

        case WM_CONTEXTMENU:
        {
            if ((HWND)wParam == listBox)
            {
                if(MessageBox(hwnd, "Do you want to delete this item?", "Attention!", MB_YESNO) == IDYES)
                {
                    int index = SendMessage(GetDlgItem(hwnd, IDC_LISTBOX), LB_GETCURSEL, 0, 0);
                    SendMessage(GetDlgItem(hwnd, IDC_LISTBOX), LB_DELETESTRING, index, 0);
                }
                break;

            }
            break;
        }

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case ID_EXIT:
                {
                    exit(1);
                }

                case ID_BCKG:
                {
                    srand(time(NULL));
                    a = rand() % 255 + 1;
                    b = rand() % 255 + 1;
                    c = rand() % 255 + 1;
                    backgFlag = true;
                    if(backgFlag == true)
                    {
                        SetClassLong(hwnd, GCL_HBRBACKGROUND, (LONG)CreateSolidBrush(RGB(a, b, c)));
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }

                case ID_INFO:
                {
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), hwnd, InfoDlgProc);
                    break;
                }

                case IDC_ADD:
                {
                    textSize = SendMessage(submitBox, WM_GETTEXT, 100, (LPARAM)textStore);
                    textStore[textSize] = _T('\0');
                    SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)textStore);
                    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
                    SendMessage(submitBox, WM_SETTEXT, NULL, NULL);
                    break;
                }
                case IDC_LISTBOX:
                {
                    if(HIWORD(wParam) == LBN_DBLCLK)
                    {
                        int index = SendMessage(GetDlgItem(hwnd, IDC_LISTBOX), LB_GETCURSEL, 0, 0);
                        int textLength = SendMessage(listBox, LB_GETTEXTLEN, (WPARAM)index, 0);
                        TCHAR * buffer = new TCHAR[textLength + 1];
                        buffer[textLength + 1] = _T('\0');
                        SendMessage(listBox, LB_GETTEXT, (WPARAM)index, (LPARAM)buffer);
                        char buffer1[50] = "You select \0";
                        char * buffer2 = new char[textLength + 1 + strlen(buffer1) + 20];
                        buffer2[0] = '\0';
                        strcat(buffer2, buffer1); strcat(buffer2, buffer);
                        MessageBox(NULL, buffer2, "Info", MB_OK);
                        delete [] buffer;
                        delete [] buffer2;
                    }
                    break;
                }
                break;
            }
            break;
        }

        case WM_SIZE:
        {
            cxCoord = LOWORD(lParam);
            cyCoord = HIWORD(lParam);
            MoveWindow(submitBox, cxCoord/18, cyCoord/20, (cxCoord - cxCoord/12 - 4*cxCoord/18), cyCoord/13, TRUE);
            MoveWindow(buttonAdd, (cxCoord - cxCoord/12 - cxCoord/18), cyCoord/20, (cxCoord/12), cyCoord/13, TRUE);
            MoveWindow(scrollBarColor, cxCoord/2+175, cyCoord/2-75, 20, cyCoord/2+40, TRUE);
            MoveWindow(listBox, cxCoord/18, cyCoord/2-75, (cxCoord - cxCoord/12 - 4*cxCoord/18), cyCoord/2+50, TRUE);
            break;
        }

        case WM_GETMINMAXINFO:
            {
                LPMINMAXINFO winSize = (LPMINMAXINFO)lParam;
                winSize->ptMinTrackSize.x = 400;
                winSize->ptMinTrackSize.y = 300;
                winSize->ptMaxTrackSize.x = 600;
                winSize->ptMaxTrackSize.y = 450;
                break;
            }
        case WM_VSCROLL:
        {
            if((HWND)lParam == scrollBarColor)
            {
                switch(LOWORD(wParam))
                {
                    case SB_LINEDOWN:
                    {
                        xPos -= 1;
                        break;
                    }

                    case SB_LINEUP:
                    {
                        xPos += 1;
                        break;
                    }

                    case SB_PAGEDOWN:
                    {
                        xPos -= 10;
                        break;
                    }

                    case SB_PAGEUP:
                    {
                        xPos += 10;
                        break;
                    }

                    case SB_TOP:
                    {
                        xPos = xMin;
                        break;
                    }

                    case SB_BOTTOM:
                    {
                        xPos = xMax;
                        break;
                    }

                    case SB_THUMBPOSITION:
                    {
                        break;
                    }

                    case SB_THUMBTRACK:
                    {
                        xPos = HIWORD(wParam);
                        break;
                    }

                    default:
                        break;


                }
                SetScrollPos(scrollBarColor, SB_CTL, xPos, TRUE);
                if(xPos == xMax)
                {
                    EnableScrollBar(scrollBarColor, SB_CTL, ESB_DISABLE_RIGHT);
                }
                if(xPos == xMin)
                {
                    EnableScrollBar(scrollBarColor, SB_CTL, ESB_DISABLE_LEFT);
                }
                InvalidateRect(submitBox, NULL, TRUE);
            }
            break;
        }
        case WM_CTLCOLOREDIT:
        {
            SetTextColor((HDC)wParam, RGB(xPos+1, 100, 255-xPos));
            SetBkMode((HDC)wParam, RGB(255, 0, 100));
            HBRUSH locBrush=(HBRUSH)CreateSolidBrush(RGB(255, 255, 255));
            return(LRESULT) locBrush;
        }

        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT)
            {
                SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(ID_CURSOR)));
                return TRUE;
            }
            break;
        }

        case WM_HOTKEY:
        {
            switch(wParam)
            {
                case HK_EXIT:
                {
                    PostQuitMessage(0);
                    break;
                }

                case HK_INFO:
                {
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), hwnd, InfoDlgProc);
                    break;
                }

                default:
                    break;
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

BOOL CALLBACK InfoDlgProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return TRUE;
            }
    }
    return FALSE;
}
