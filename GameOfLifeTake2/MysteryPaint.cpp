#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include <vector>
#pragma comment(lib, "d2d1")

#include "basewin.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

class MainWindow : public BaseWindow<MainWindow>
{
    ID2D1Factory            *pFactory;
    ID2D1HwndRenderTarget   *pRenderTarget;
    ID2D1SolidColorBrush    *pBrush;
    std::vector<D2D1_RECT_F> rectVect;
    std::vector<float> ellipseColorVector;
    ID2D1GeometrySink        *pSink;

    bool                    draw =false;
    int                     brushSize = 30;
    bool                    flag = false;

    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    HRESULT NewGraphicsResource(float r, float g, float b, float x_pos, float y_pos);
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    CalculateNewLayout(float xpos, float ypos);
    void    Resize();

public:

    MainWindow() : pFactory(NULL), pRenderTarget(NULL), pBrush(NULL)
    {
    }

    PCWSTR  ClassName() const { return L"Circle Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// Recalculate drawing layout when the size of the window changes.

void MainWindow::CalculateLayout()
{
    if (pRenderTarget != NULL)
    {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        const float x = size.width;
        const float y = size.height;
        const float radius = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))*4;
        rectVect.push_back(D2D1::RectF(x, y));
    }
}

void MainWindow::CalculateNewLayout(float xpos, float ypos)
{
    if (pRenderTarget != NULL)
    {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        const float x = size.width / 2;
        const float y = size.height / 2;
        const float radius = 15+(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 60;
        rectVect.push_back(D2D1::RectF(xpos+brushSize, ypos+ brushSize, xpos - brushSize, ypos - brushSize));
    }
}

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::AliceBlue);
            ellipseColorVector.push_back(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
            ellipseColorVector.push_back(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
            ellipseColorVector.push_back(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

            if (SUCCEEDED(hr))
            {
                CalculateLayout();
            }
        }
    }
    return hr;
}

HRESULT MainWindow::NewGraphicsResource(float r, float g, float b, float x_pos, float y_pos)
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(r, g, b);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
            if (SUCCEEDED(hr))
            {
                CalculateNewLayout(x_pos, y_pos);
            }
        }
    }
    return hr;
}
void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
     
        pRenderTarget->BeginDraw();

        pRenderTarget->Clear( D2D1::ColorF(D2D1::ColorF::AliceBlue) );
        
        for (int i = 0; i < rectVect.size(); i++) {
            pBrush->SetColor(D2D1::ColorF(ellipseColorVector[i], ellipseColorVector[i+1], ellipseColorVector[i+2], 0.7f));
            pRenderTarget->FillRectangle(rectVect[i], pBrush);

        }
        
        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

void MainWindow::Resize()
{
    if (pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    MainWindow win;

    if (!win.Create(L"Paint but Worse", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            return -1;  // Fail CreateWindowEx.
        }
        return 0;

    case WM_CLOSE:
    {
        if (MessageBox(m_hwnd, L"Really quit?", L"Paint but Worse", MB_OKCANCEL) == IDOK)
        {
            DestroyWindow(m_hwnd);
        }
        return 0;
    }
    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
    {
        DiscardGraphicsResources();
        float x = GET_X_LPARAM(lParam);
        float y = GET_Y_LPARAM(lParam);
        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        ellipseColorVector.push_back(r);
        ellipseColorVector.push_back(g);
        ellipseColorVector.push_back(b);
        NewGraphicsResource(r, g, b, x, y);
        OnPaint();
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (DragDetect(m_hwnd, pt))
        {
            draw = true;
        }
        return 0;

    }
    case WM_KEYDOWN:
        if (wParam == VK_RETURN) {
            DiscardGraphicsResources();
            float x = GET_X_LPARAM(lParam);
            float y = GET_Y_LPARAM(lParam);

            float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            ellipseColorVector.push_back(r);
            ellipseColorVector.push_back(g);
            ellipseColorVector.push_back(b);
            rectVect.clear();
            ellipseColorVector.clear();
            CreateGraphicsResources();
            NewGraphicsResource(r, g, b, x, y);
            OnPaint();
        }
        else if (wParam == VK_SUBTRACT) {
            brushSize -= 1;
            
            if (brushSize < 0) {
                brushSize = 1;
            }
        }
        else if (wParam == VK_ADD) {
            brushSize += 1;
        }
        return 0;

    case WM_MOUSEMOVE:
        
        if (draw) {
            DiscardGraphicsResources();
            float x = GET_X_LPARAM(lParam);
            float y = GET_Y_LPARAM(lParam);
            float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            ellipseColorVector.push_back(r);
            ellipseColorVector.push_back(g);
            ellipseColorVector.push_back(b);
            NewGraphicsResource(r, g, b, x, y);
            OnPaint();

        }
        return 0;

    case WM_LBUTTONUP:
        draw = false;
    case WM_PAINT:
        OnPaint();
        return 0;


    case WM_SIZE:
        Resize();
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}