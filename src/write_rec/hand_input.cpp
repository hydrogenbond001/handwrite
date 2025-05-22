#include <windows.h>
#include <opencv2/opencv.hpp>
#include <main.h>
#include <iostream>
#define CANVAS_SIZE 28 * 4
#define SCALE_FACTOR 4 // 将28x28放大显示

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
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

    if (!RegisterClass(&wndclass))
    {
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

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

void CreateControls(HWND hwnd)
{
    // 创建按钮
    CreateWindow(TEXT("BUTTON"), TEXT("handwrite"),
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 CANVAS_SIZE * SCALE_FACTOR + 20, 20, 100, 30,
                 hwnd, (HMENU)1, NULL, NULL);

    CreateWindow(TEXT("BUTTON"), TEXT("handwrite recognize"),
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 CANVAS_SIZE * SCALE_FACTOR + 20, 60, 100, 30,
                 hwnd, (HMENU)2, NULL, NULL);

    CreateWindow(TEXT("BUTTON"), TEXT("screen cut"),
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 CANVAS_SIZE * SCALE_FACTOR + 20, 100, 100, 30,
                 hwnd, (HMENU)3, NULL, NULL);

    CreateWindow(TEXT("BUTTON"), TEXT("Clear"),
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 CANVAS_SIZE * SCALE_FACTOR + 20, 140, 100, 30,
                 hwnd, (HMENU)4, NULL, NULL);
}

void DrawCanvas(HDC hdc)
{
    RECT rect;
    HBRUSH hBrush;

    // 绘制背景
    SetRect(&rect, 0, 0, CANVAS_SIZE * SCALE_FACTOR, CANVAS_SIZE * SCALE_FACTOR);
    FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // 绘制网格线
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    for (int i = 0; i <= CANVAS_SIZE; i++)
    {
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
    for (int y = 0; y < CANVAS_SIZE; y++)
    {
        for (int x = 0; x < CANVAS_SIZE; x++)
        {
            if (canvas[y][x])
            {
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
cv::Mat g_screen, g_temp;
cv::Rect g_rect;
bool drawing = false;
cv::Point startPoint;
cv::Mat captureScreen()
{
    HWND hwndDesktop = GetDesktopWindow();
    HDC hdcWindow = GetDC(hwndDesktop);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);
    SelectObject(hdcMemDC, hBitmap);
    BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    cv::Mat mat(height, width, CV_8UC4);
    GetBitmapBits(hBitmap, bmp.bmHeight * bmp.bmWidthBytes, mat.data);

    DeleteObject(hBitmap);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwndDesktop, hdcWindow);

    cv::Mat mat_bgr;
    cv::cvtColor(mat, mat_bgr, cv::COLOR_BGRA2BGR);
    return mat_bgr;
}

void mouseCallback(int event, int x, int y, int, void *userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        drawing = true;
        startPoint = cv::Point(x, y);
        g_temp = g_screen.clone();
    }
    else if (event == cv::EVENT_MOUSEMOVE && drawing)
    {
        g_temp = g_screen.clone();
        cv::rectangle(g_temp, startPoint, cv::Point(x, y), cv::Scalar(0, 255, 0), 2);
        cv::imshow("Select Region", g_temp);
    }
    else if (event == cv::EVENT_LBUTTONUP && drawing)
    {
        drawing = false;
        g_rect = cv::Rect(startPoint, cv::Point(x, y));
        cv::destroyWindow("Select Region");
        *static_cast<bool *>(userdata) = true;
    }
}

void manualScreenshotAndOCR()
{
    g_screen = captureScreen();
    g_temp = g_screen.clone();

    bool finished = false;
    cv::namedWindow("Select Region", cv::WINDOW_NORMAL);
    cv::setWindowProperty("Select Region", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    cv::setMouseCallback("Select Region", mouseCallback, &finished);
    cv::imshow("Select Region", g_temp);

    while (!finished)
    {
        cv::waitKey(30);
    }

    if (g_rect.width > 0 && g_rect.height > 0)
    {
        cv::Mat roi = g_screen(g_rect);
        std::string result = runOCR(roi); // 直接调用全局 OCR 函数
        std::cout << "OCR Result: " << result << std::endl;

        cv::imshow("Selected Region", roi);
        cv::waitKey(2000);
        cv::destroyAllWindows();
    }
    else
    {
        std::cout << "未选中区域，取消 OCR。" << std::endl;
    }
}

void ClearCanvas()
{
    for (int y = 0; y < CANVAS_SIZE; y++)
    {
        for (int x = 0; x < CANVAS_SIZE; x++)
        {
            canvas[y][x] = 0;
        }
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    static int prevX = -1, prevY = -1;

    switch (message)
    {
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
        if (isDrawing)
        {
            int x = LOWORD(lParam) / SCALE_FACTOR;
            int y = HIWORD(lParam) / SCALE_FACTOR;

            if (x >= 0 && x < CANVAS_SIZE && y >= 0 && y < CANVAS_SIZE)
            {
                // 绘制当前点
                canvas[y][x] = 1;

                // 如果之前有点，绘制连线
                if (prevX != -1 && prevY != -1)
                {
                    int dx = abs(x - prevX);
                    int dy = abs(y - prevY);
                    int sx = prevX < x ? 1 : -1;
                    int sy = prevY < y ? 1 : -1;
                    int err = (dx > dy ? dx : -dy) / 2;

                    while (1)
                    {
                        if (prevX >= 0 && prevX < CANVAS_SIZE && prevY >= 0 && prevY < CANVAS_SIZE)
                        {
                            canvas[prevY][prevX] = 1;
                        }

                        if (prevX == x && prevY == y)
                            break;
                        int e2 = err;
                        if (e2 > -dx)
                        {
                            err -= dy;
                            prevX += sx;
                        }
                        if (e2 < dy)
                        {
                            err += dx;
                            prevY += sy;
                        }
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
        switch (LOWORD(wParam))
        {
        case 1:
        {
            // 创建一个 OpenCV 图像，大小为 28x28，单通道（灰度）CANVAS_SIZE
            cv::Mat img(CANVAS_SIZE, CANVAS_SIZE, CV_8UC1);

            // 填充像素
            for (int y = 0; y < CANVAS_SIZE; y++)
            {
                for (int x = 0; x < CANVAS_SIZE; x++)
                {
                    img.at<uchar>(y, x) = canvas[y][x] ? 255 : 0; // 黑白反转：1变255，0变0
                }
            }
            cv::resize(img, img, cv::Size(280, 280));
            // 显示图像（注意：必须是控制台程序或 GUI 程序中启用了cv窗口功能）
            cv::destroyAllWindows();
            cv::imshow("Canvas Image", img);
            cv::waitKey(3000); // 允许 OpenCV 窗口刷新
            cv::destroyAllWindows();
            break;
        }

        case 2:
        {
            cv::Mat img2(CANVAS_SIZE, CANVAS_SIZE, CV_8UC1);

            // 填充像素
            for (int y = 0; y < CANVAS_SIZE; y++)
            {
                for (int x = 0; x < CANVAS_SIZE; x++)
                {
                    img2.at<uchar>(y, x) = canvas[y][x] ? 0 : 255;
                }
            }
            // 放大显示用
            cv::resize(img2, img2, cv::Size(280, 280));

            // 转换为 RGB 三通道
            cv::Mat imgRGB;
            cv::cvtColor(img2, imgRGB, cv::COLOR_GRAY2RGB);

            // 显示图像
            cv::imshow("Canvas Image", imgRGB);

            // OCR 识别
            std::string ocrResult = runOCR(imgRGB);
            std::cout << "OCR Result: " << ocrResult << std::endl;

            cv::waitKey(3000);
            cv::destroyAllWindows();
            break;
        }
        case 3:
        {
            manualScreenshotAndOCR();
            break;
        }

        case 4:
            std::cout << "case 4" << std::endl;
            ClearCanvas();
            break;
        }
        return 0;

    case WM_DESTROY:
        if (hBitmap)
            DeleteObject(hBitmap);
        PostQuitMessage(0);

        // cv::destroyAllWindows();
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}