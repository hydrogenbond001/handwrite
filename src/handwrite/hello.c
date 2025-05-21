#include <windows.h>

#define CANVAS_SIZE 28
#define SCALE_FACTOR 15  // 将28x28放大显示

// 全局变量
HWND hwnd;
HBITMAP hBitmap = NULL;
int canvas[CANVAS_SIZE][CANVAS_SIZE] = {0};
int isDrawing = 0;

// 函数声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateControls(HWND);
void DrawCanvas(HDC hdc);
void ClearCanvas();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
    static TCHAR szAppName[] = TEXT("Handwriting Demo");
    MSG msg;
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass)) {
        MessageBox(NULL, TEXT("Program requires Windows NT!"), szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName,
                        TEXT("28x28 Handwriting Canvas"),
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CANVAS_SIZE * SCALE_FACTOR + 150,
                        CANVAS_SIZE * SCALE_FACTOR + 50,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);

    CreateControls(hwnd);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

void CreateControls(HWND hwnd) {
    // 创建按钮
    CreateWindow(TEXT("BUTTON"), TEXT("Button 1"), 
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                CANVAS_SIZE * SCALE_FACTOR + 20, 20, 100, 30,
                hwnd, (HMENU)1, NULL, NULL);
    
    CreateWindow(TEXT("BUTTON"), TEXT("Button 2"), 
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                CANVAS_SIZE * SCALE_FACTOR + 20, 60, 100, 30,
                hwnd, (HMENU)2, NULL, NULL);
    
    CreateWindow(TEXT("BUTTON"), TEXT("Button 3"), 
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                CANVAS_SIZE * SCALE_FACTOR + 20, 100, 100, 30,
                hwnd, (HMENU)3, NULL, NULL);
    
    CreateWindow(TEXT("BUTTON"), TEXT("Clear"), 
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                CANVAS_SIZE * SCALE_FACTOR + 20, 140, 100, 30,
                hwnd, (HMENU)4, NULL, NULL);
}

void DrawCanvas(HDC hdc) {
    RECT rect;
    HBRUSH hBrush;
    
    // 绘制背景
    SetRect(&rect, 0, 0, CANVAS_SIZE * SCALE_FACTOR, CANVAS_SIZE * SCALE_FACTOR);
    FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    
    // 绘制网格线
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    HPEN hOldPen = SelectObject(hdc, hPen);
    
    for (int i = 0; i <= CANVAS_SIZE; i++) {
        // 垂直线
        MoveToEx(hdc, i * SCALE_FACTOR, 0, NULL);
        LineTo(hdc, i * SCALE_FACTOR, CANVAS_SIZE * SCALE_FACTOR);
        
        // 水平线
        MoveToEx(hdc, 0, i * SCALE_FACTOR, NULL);
        LineTo(hdc, CANVAS_SIZE * SCALE_FACTOR, i * SCALE_FACTOR);
    }
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    // 绘制像素
    hBrush = CreateSolidBrush(RGB(0, 0, 0));
    for (int y = 0; y < CANVAS_SIZE; y++) {
        for (int x = 0; x < CANVAS_SIZE; x++) {
            if (canvas[y][x]) {
                rect.left = x * SCALE_FACTOR;
                rect.top = y * SCALE_FACTOR;
                rect.right = (x + 1) * SCALE_FACTOR;
                rect.bottom = (y + 1) * SCALE_FACTOR;
                FillRect(hdc, &rect, hBrush);
            }
        }
    }
    DeleteObject(hBrush);
}

void ClearCanvas() {
    for (int y = 0; y < CANVAS_SIZE; y++) {
        for (int x = 0; x < CANVAS_SIZE; x++) {
            canvas[y][x] = 0;
        }
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    static int prevX = -1, prevY = -1;
    
    switch (message) {
        case WM_CREATE:
            return 0;
            
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            DrawCanvas(hdc);
            EndPaint(hwnd, &ps);
            return 0;
            
        case WM_LBUTTONDOWN:
            isDrawing = 1;
            // 继续到 WM_MOUSEMOVE
            
        case WM_MOUSEMOVE:
            if (isDrawing) {
                int x = LOWORD(lParam) / SCALE_FACTOR;
                int y = HIWORD(lParam) / SCALE_FACTOR;
                
                if (x >= 0 && x < CANVAS_SIZE && y >= 0 && y < CANVAS_SIZE) {
                    // 绘制当前点
                    canvas[y][x] = 1;
                    
                    // 如果之前有点，绘制连线
                    if (prevX != -1 && prevY != -1) {
                        int dx = abs(x - prevX);
                        int dy = abs(y - prevY);
                        int sx = prevX < x ? 1 : -1;
                        int sy = prevY < y ? 1 : -1;
                        int err = (dx > dy ? dx : -dy) / 2;
                        
                        while (1) {
                            if (prevX >= 0 && prevX < CANVAS_SIZE && prevY >= 0 && prevY < CANVAS_SIZE) {
                                canvas[prevY][prevX] = 1;
                            }
                            
                            if (prevX == x && prevY == y) break;
                            int e2 = err;
                            if (e2 > -dx) { err -= dy; prevX += sx; }
                            if (e2 < dy) { err += dx; prevY += sy; }
                        }
                    }
                    
                    prevX = x;
                    prevY = y;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
            
        case WM_LBUTTONUP:
            isDrawing = 0;
            prevX = prevY = -1;
            return 0;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1:
                    // MessageBox(hwnd, TEXT("Button 1 clicked"), TEXT("Info"), MB_OK);
                    break;
                case 2:
                    // MessageBox(hwnd, TEXT("Button 2 clicked"), TEXT("Info"), MB_OK);
                    break;
                case 3:
                    // MessageBox(hwnd, TEXT("Button 3 clicked"), TEXT("Info"), MB_OK);
                    break;
                case 4:
                    ClearCanvas();
                    break;
            }
            return 0;
            
        case WM_DESTROY:
            if (hBitmap) DeleteObject(hBitmap);
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, message, wParam, lParam);
}