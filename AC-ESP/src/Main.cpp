#include <windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <iostream>
#include <vector>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include "ProccessPlayers.cpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param)) {
        return 0L;
    }
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0L;
    }
    return DefWindowProc(window, message, w_param, l_param);
}

HWND FindGameWindow(const wchar_t* windowName) {
    return FindWindow(NULL, windowName);
}

void GetWindowSizeAndPosition(HWND hwnd, RECT& rect) {
    GetWindowRect(hwnd, &rect);
}

void ApplyWindowAttributes(HWND gameWindow, HWND overlayWindow) {
    RECT gameRect;
    GetWindowSizeAndPosition(gameWindow, gameRect);

    SetWindowPos(overlayWindow, HWND_TOPMOST, gameRect.left, gameRect.top,
        gameRect.right - gameRect.left, gameRect.bottom - gameRect.top,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);

    SetLayeredWindowAttributes(overlayWindow, RGB(0, 0, 0), 0, LWA_COLORKEY);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(overlayWindow, &margins);
}

INT WINAPI WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = instance;
    wc.lpszClassName = L"AC-ESP";
    FVP FVP;
    RegisterClassExW(&wc);

    HWND gameWindow = FindGameWindow(L"AssaultCube");
    if (!gameWindow) {
        std::cout << "Failed to find AssaultCube window." << std::endl;
        return 1;
    }
    RECT gameRect;
    GetWindowSizeAndPosition(gameWindow, gameRect);
    FVP.SetScreenDimensions(gameRect.right - gameRect.left, gameRect.bottom - gameRect.top);

    const HWND window = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        wc.lpszClassName,
        L"AC-ESP",
        WS_POPUP,
        gameRect.left,
        gameRect.top,
        FVP.screenWidth,
        FVP.screenHeight,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    ApplyWindowAttributes(gameWindow, window);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.RefreshRate.Numerator = 60U;
    sd.BufferDesc.RefreshRate.Denominator = 1U;
    sd.BufferCount = 2U;
    sd.SampleDesc.Count = 1U;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = window;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL levels[2]{
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    ID3D11Device* device{ nullptr };
    ID3D11DeviceContext* device_context{ nullptr };
    IDXGISwapChain* swap_chain{ nullptr };
    ID3D11RenderTargetView* render_target_view{ nullptr };
    D3D_FEATURE_LEVEL level{};

    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0U,
        levels,
        2U,
        D3D11_SDK_VERSION,
        &sd,
        &swap_chain,
        &device,
        &level,
        &device_context
    );

    ID3D11Texture2D* back_buffer{ nullptr };
    swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

    if (back_buffer) {
        device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
        back_buffer->Release();
    }
    else {
        return 1;
    }

    ShowWindow(window, cmd_show);
    UpdateWindow(window);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplDX11_Init(device, device_context);
    ImGui_ImplWin32_Init(window);

    bool running = true;

    struct ScreenPos {
        memory_manager::Vec2 head;
        memory_manager::Vec2 feet;
    };
    struct Boxes {
        int boxHeight;
        int boxWidth;
        int boxXmin;
        int boxYmin;
        int boxXmax;
        int boxYmax;
    };

    ImU32 col = IM_COL32(255, 0, 0, 255); // Red color
    float rounding = 0.0f;
    ImDrawFlags flags = 0;
    float thickness = 1.0f;

    while (running) {
        MSG message;
        while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
            if (message.message == WM_QUIT) {
                running = false;
            }
        }

        if (!running) {
            break;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();
        std::vector<Boxes> BoxesToDraw;

        std::vector<memory_manager::Player> players = FVP.loop();

        for (auto& player : players) {
            ScreenPos screenPos;
            screenPos.feet = player.feet_screen_pos;
            screenPos.head = player.head_screen_pos;

            Boxes boxes;
            boxes.boxHeight = int(screenPos.feet.y - screenPos.head.y);
            boxes.boxWidth = int(boxes.boxHeight / 3);
            boxes.boxXmin = int(screenPos.head.x - boxes.boxWidth / 2);
            boxes.boxYmin = int(screenPos.head.y);
            boxes.boxXmax = (boxes.boxXmin + boxes.boxWidth);
            boxes.boxYmax = (boxes.boxYmin + boxes.boxHeight);
            BoxesToDraw.push_back(boxes);
        }

        for (auto& boxes : BoxesToDraw) {
            ImVec2 p_min(boxes.boxXmin, boxes.boxYmin); // Top-left corner
            ImVec2 p_max(boxes.boxXmax, boxes.boxYmax); // Bottom-right corner

            ImGui::GetBackgroundDrawList()->AddRect(
                p_min,
                p_max,
                col,
                rounding,
                flags,
                thickness
            );
        }

        ImGui::Render();

        constexpr FLOAT color[4]{ 0.f, 0.f, 0.f, 0.f };
        if (render_target_view) {
            device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
            device_context->ClearRenderTargetView(render_target_view, color);
        }

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(1U, 0U);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (swap_chain) {
        swap_chain->Release();
    }

    if (device) {
        device->Release();
    }

    if (device_context) {
        device_context->Release();
    }

    if (render_target_view) {
        render_target_view->Release();
    }

    DestroyWindow(window);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}
