#include <windows.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <main.h>
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 300

// 全局变量
HWND hwndMain;
HWND hwndResultEdit;
cv::Mat g_screen, g_temp;
cv::Rect g_rect;
bool g_drawing = false;
cv::Point g_startPoint;
bool g_finishedSelection = false;
HFONT hFont;

// 函数声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateUI(HWND hwnd);
void CaptureScreen();
void OnScreenshot();
std::string RunOCR(const cv::Mat &image);
void DisplayResult(const std::string &text);

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    SetProcessDPIAware();                        // Windows高DPI适配
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE); //  隐藏控制台窗口
    // 注册窗口类
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = TEXT("ScreenOCR");

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error"), MB_ICONERROR);
        return 0;
    }

    // 创建窗口
    hwndMain = CreateWindow(
        TEXT("ScreenOCR"),
        TEXT("屏幕截图OCR识别工具"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL);

    if (!hwndMain)
    {
        MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error"), MB_ICONERROR);
        return 0;
    }

    // 创建UI控件
    CreateUI(hwndMain);

    // 显示窗口
    ShowWindow(hwndMain, iCmdShow);
    UpdateWindow(hwndMain);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// 创建UI控件
void CreateUI(HWND hwnd)
{
    // 创建截图按钮
    CreateWindow(TEXT("BUTTON"), TEXT("截图识别"),
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 20, 20, 150, 40,
                 hwnd, (HMENU)1, NULL, NULL);

    // 创建结果文本框
    hwndResultEdit = CreateWindow(TEXT("EDIT"), NULL,
                                  WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL,
                                  20, 80, WINDOW_WIDTH - 60, WINDOW_HEIGHT - 120,
                                  hwnd, (HMENU)2, NULL, NULL);

    // 创建字体
    hFont = CreateFont(18 * 1.5, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                       OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                       VARIABLE_PITCH, TEXT("微软雅黑"));

    // 设置文本框字体
    SendMessage(hwndResultEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
}

// 截图回调函数
void ScreenshotCallback(int event, int x, int y, int, void *)
{
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        g_drawing = true;
        g_startPoint = cv::Point(x, y);
        g_temp = g_screen.clone();
    }
    else if (event == cv::EVENT_MOUSEMOVE && g_drawing)
    {
        g_temp = g_screen.clone();
        cv::rectangle(g_temp, g_startPoint, cv::Point(x, y), cv::Scalar(0, 255, 0), 2);
        cv::imshow("请选择要识别的区域 (松开鼠标完成选择)", g_temp);
    }
    else if (event == cv::EVENT_LBUTTONUP && g_drawing)
    {
        g_drawing = false;
        g_rect = cv::Rect(g_startPoint, cv::Point(x, y));
        g_finishedSelection = true;
        cv::destroyWindow("请选择要识别的区域 (松开鼠标完成选择)");
    }
}

// 捕获屏幕
void CaptureScreen()
{
    HWND hwndDesktop = GetDesktopWindow();
    HDC hdcWindow = GetDC(hwndDesktop);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

    int width = GetSystemMetrics(SM_CXSCREEN) /** 1.5*/; // SetProcessDPIAware(); // Windows高DPI适配
    int height = GetSystemMetrics(SM_CYSCREEN) /** 1.5*/;

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

    cv::cvtColor(mat, g_screen, cv::COLOR_BGRA2BGR);
}

// 截图处理
void OnScreenshot()
{
    // 捕获屏幕
    CaptureScreen();
    g_temp = g_screen.clone();
    g_finishedSelection = false;

    // 创建全屏窗口让用户选择区域
    cv::namedWindow("请选择要识别的区域 (松开鼠标完成选择)", cv::WINDOW_NORMAL);
    cv::setWindowProperty("请选择要识别的区域 (松开鼠标完成选择)", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    cv::setMouseCallback("请选择要识别的区域 (松开鼠标完成选择)", ScreenshotCallback);

    // 显示屏幕截图
    cv::imshow("请选择要识别的区域 (松开鼠标完成选择)", g_temp);

    // 等待用户选择完成
    while (!g_finishedSelection)
    {
        if (cv::waitKey(10) == 27)
        {
            cv::destroyAllWindows();
            g_drawing = false;
        }
    }

    // 检查是否选择了有效区域
    if (g_rect.width > 0 && g_rect.height > 0)
    {
        // 提取选定区域
        cv::Mat roi = g_screen(g_rect);
        // roi = PreprocessImage(roi);
        // 运行OCR识别
        std::string result = RunOCR(roi);

        // 显示识别结果
        DisplayResult(result);

        // 显示选定的区域
        // cv::imshow("选定区域", roi);
        // cv::waitKey(2000);
        // cv::destroyAllWindows();
    }
    else
    {
        DisplayResult("未选择有效区域，请重试。");
    }
}

// OCR识别函数 (模拟实现)
std::string RunOCR(const cv::Mat &image)
{
    // 这里应该调用实际的OCR引擎
    // 例如: Tesseract、百度OCR、腾讯OCR等
    std::string ocrResult = runOCR(image);
    // 模拟实现 - 实际应用中应该替换为真正的OCR调用
    return ocrResult;
}

// 显示识别结果
void DisplayResult(const std::string &text)
{
    // 参数检查
    if (hwndResultEdit == NULL || text.empty())
    {
        return;
    }

    // 转换编码为宽字符
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
    if (len <= 0)
    {
        return;
    }

    wchar_t *wstr = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wstr, len);

    // 获取当前文本长度（避免超过最大限制）
    int currentLength = GetWindowTextLength(hwndResultEdit);
    const int maxLength = 32000; // EDIT 控件的最大容量

    // 自动换行处理（每60个字符插入换行符）
    std::wstring formattedText;
    int lineLength = 0;
    const int maxLineLength = 20; // 每行最大字符数

    for (int i = 0; wstr[i] != L'\0'; ++i)
    {
        formattedText += wstr[i];
        lineLength++;

        // 达到最大行长度时换行（不拆分单词）
        if (lineLength >= maxLineLength && std::iswspace(wstr[i]))
        {
            formattedText += L"\r\n";
            lineLength = 0;
        }
    }

    // 如果文本框内容过长，清空前半部分
    if (currentLength + formattedText.length() > maxLength * 0.8)
    {
        SetWindowText(hwndResultEdit, L"");
    }

    // 追加新文本到文本框
    SendMessage(hwndResultEdit, EM_SETSEL, (WPARAM)currentLength, (LPARAM)currentLength);
    SendMessage(hwndResultEdit, EM_REPLACESEL, TRUE, (LPARAM)formattedText.c_str());

    // 滚动到最新内容
    SendMessage(hwndResultEdit, WM_VSCROLL, SB_BOTTOM, 0);

    delete[] wstr;
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1: // 截图按钮
            OnScreenshot();
            break;
        }
        break;

    case WM_DESTROY:
        if (hFont)
            DeleteObject(hFont);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}