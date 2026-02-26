#include "overlay.h"
#include "globals.h"
#include "features/esp.h"
#include "features/aimbot.h"
#include "features/makcu.h"
#include "ui_style.h"

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

// Forward declare per WndProc di ImGui
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// D3D11 state
static HWND                    s_hwnd = nullptr;
static ID3D11Device*           s_device = nullptr;
static ID3D11DeviceContext*    s_context = nullptr;
static IDXGISwapChain*         s_swapChain = nullptr;
static ID3D11RenderTargetView* s_rtv = nullptr;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_KEYDOWN:
        if (wParam == VK_INSERT) g::menuOpen = !g::menuOpen;
        if (wParam == VK_END)    g::running = false;
        break;
    case WM_DESTROY:
        g::running = false;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

bool overlay::Create(int width, int height)
{
    // --- Registra window class ---
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = GetModuleHandleA(nullptr);
    wc.lpszClassName  = "CS2_DMA_ESP_Overlay";
    wc.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassExA(&wc);

    // --- Crea finestra (borderless, nera) ---
    s_hwnd = CreateWindowExA(
        0,
        wc.lpszClassName,
        "CS2 DMA ESP",
        WS_POPUP | WS_VISIBLE,
        0, 0, width, height,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!s_hwnd) return false;

    ShowWindow(s_hwnd, SW_SHOW);
    UpdateWindow(s_hwnd);

    // --- Init D3D11 ---
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount       = 1;
    sd.BufferDesc.Width  = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = s_hwnd;
    sd.SampleDesc.Count   = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed     = TRUE;
    sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &sd, &s_swapChain, &s_device, &featureLevel, &s_context);

    if (FAILED(hr)) return false;

    // --- Render target ---
    ID3D11Texture2D* backBuffer = nullptr;
    s_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    s_device->CreateRenderTargetView(backBuffer, nullptr, &s_rtv);
    backBuffer->Release();

    // --- Init ImGui ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Load Segoe UI font (fallback to default if not found)
    const char* fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
    if (GetFileAttributesA(fontPath) != INVALID_FILE_ATTRIBUTES)
        io.Fonts->AddFontFromFileTTF(fontPath, 16.0f);

    ui::ApplyStyle();

    ImGui_ImplWin32_Init(s_hwnd);
    ImGui_ImplDX11_Init(s_device, s_context);

    return true;
}

void overlay::Run()
{
    MSG msg = {};
    while (g::running) {
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
            if (msg.message == WM_QUIT)
                g::running = false;
        }
        if (!g::running) break;

        // --- Leggi dati dal gioco via DMA ---
        esp::UpdateData();

        // --- Aimbot (dopo UpdateData, prima del rendering) ---
        aimbot::Run();

        // --- Begin ImGui frame ---
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // --- Disegna ESP + Radar + FOV circle ---
        esp::Draw();
        aimbot::DrawFovCircle();

        // --- Disegna menu centralizzato ---
        ui::RenderMenu();

        // --- Render ---
        ImGui::Render();

        const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        s_context->OMSetRenderTargets(1, &s_rtv, nullptr);
        s_context->ClearRenderTargetView(s_rtv, clearColor);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        s_swapChain->Present(1, 0); // VSync on
    }
}

void overlay::Destroy()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (s_rtv)       s_rtv->Release();
    if (s_swapChain) s_swapChain->Release();
    if (s_context)   s_context->Release();
    if (s_device)    s_device->Release();

    if (s_hwnd) DestroyWindow(s_hwnd);
}
