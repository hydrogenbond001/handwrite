#include <windows.h>
#include <opencv2/opencv.hpp>
#include <main.h>
#include <iostream>

#define CANVAS_SIZE 28*4
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

cv::Mat getImageFromClipboard()
{
    if (!OpenClipboard(nullptr))
    {
        std::cerr << "Failed to open clipboard." << std::endl;
        return {};
    }

    HANDLE hData = GetClipboardData(CF_DIB);
    if (!hData)
    {
        CloseClipboard();
        std::cerr << "No image in clipboard." << std::endl;
        return {};
    }

    BITMAPINFO *bmpInfo = (BITMAPINFO *)GlobalLock(hData);
    if (!bmpInfo)
    {
        CloseClipboard();
        std::cerr << "Failed to lock image data." << std::endl;
        return {};
    }

    int width = bmpInfo->bmiHeader.biWidth;
    int height = abs(bmpInfo->bmiHeader.biHeight); // may be negative
    int channels = bmpInfo->bmiHeader.biBitCount / 8;

    BYTE *pixelData = (BYTE *)bmpInfo + bmpInfo->bmiHeader.biSize;
    cv::Mat img(height, width, (channels == 3 ? CV_8UC3 : CV_8UC4), pixelData);
    cv::Mat imgCopy = img.clone(); // Copy before unlocking

    GlobalUnlock(hData);
    CloseClipboard();
    return imgCopy;
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
            system("start ms-screenclip"); // 打开系统截图工具（Win+Shift+S）
            std::cout << "请使用鼠标截图，截图后按下回车继续..." << std::endl;
            std::cin.get(); // 等待用户截图并回车

            cv::Mat img = getImageFromClipboard();
            if (img.empty())
            {
                std::cout << "截图失败或剪贴板无图像！" << std::endl;
                break;
            }

            // OpenCV 剪贴板图像为 BGRA，转 RGB
            if (img.channels() == 4)
            {
                cv::cvtColor(img, img, cv::COLOR_BGRA2RGB);
            }
            else if (img.channels() == 3)
            {
                cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
            }

            std::string ocrResult = runOCR(img);
            std::cout << "OCR 识别结果: " << ocrResult << std::endl;

            cv::imshow("截图", img);
            cv::waitKey(3000);
            cv::destroyAllWindows();
            break;
        }

        case 4:
            ClearCanvas();
            break;
        }
        return 0;

    case WM_DESTROY:
        if (hBitmap)
            DeleteObject(hBitmap);
        PostQuitMessage(0);
        cv::destroyAllWindows();
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}