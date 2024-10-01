#include <Windows.h>
#include <iostream>

#include "Entitys.hpp"
#include "WorldToScreen.hpp"
#include "config.h"
#include "thread"

#include <d3d11.h>
#include <dxgi.h>
#include "kiero/kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "bones.h"
#include "utils.h"

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

void InitImGui()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

using namespace std;


void DrawHealthbar(ImVec2 boxMin, ImVec2 boxMax, int health) {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    float healthPercentage = static_cast<float>(health) / 100.0f;
    const float barThickness = 5.0f;
    const float offsetX = 6.0f;

    ImVec2 barBackgroundMin = ImVec2(boxMin.x - barThickness - offsetX, boxMin.y);
    ImVec2 barBackgroundMax = ImVec2(boxMin.x - offsetX, boxMax.y);
    drawList->AddRectFilled(barBackgroundMin, barBackgroundMax, IM_COL32(80, 0, 0, 255));

    float barHeight = (boxMax.y - boxMin.y) * healthPercentage;
    ImVec2 barMin = ImVec2(boxMin.x - barThickness - offsetX, boxMax.y - barHeight);
    ImVec2 barMax = ImVec2(boxMin.x - offsetX, boxMax.y);
    ImU32 healthColor = IM_COL32(255 * (1.0f - healthPercentage), 255 * healthPercentage, 0, 255);
    drawList->AddRectFilled(barMin, barMax, healthColor);

    drawList->AddRect(barBackgroundMin, barBackgroundMax, IM_COL32(255, 255, 255, 255));
}



void Trigger_bot() {
    while (true) {
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000 && config::trigger_bot) {
            const auto localPlayer = mem::Read<LONGLONG>(mem::baseAddress + offsets::dwLocalPlayerPawn);

         
            const auto playerTeam = mem::Read<int>(localPlayer + offsets::m_iTeamNum);
            const auto entityId = mem::Read<int>(localPlayer + offsets::m_iIDEntIndex);

            if (localPlayer) {
                auto health = mem::Read<LONGLONG>(localPlayer + offsets::m_iHealth);

                if (health && entityId > 0) {
                    auto entList = mem::Read<LONGLONG>(mem::baseAddress + offsets::dwEntityList);
                    auto entEntry = mem::Read<LONGLONG>(entList + 0x8 * (entityId >> 9) + 0x10);
                    auto entity = mem::Read<LONGLONG>(entEntry + 120 * (entityId & 0x1FF));
                    auto entityTeam = mem::Read<int>(entity + offsets::m_iTeamNum);

                    bool shouldShoot = true;

                    if (config::team_check) {
                        if (entityTeam == playerTeam) {
                            shouldShoot = false;
                        }
                    }

                    if (shouldShoot) {
                        auto entityHp =    mem::Read<int>(entity + offsets::m_iHealth);
                        if (entityHp > 0) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                            Sleep(14);
                        }
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(config::triggerdelay));
    }
}

void bunny_hop() {
    while (true) {
        if (config::bunny_hop)
        {

            const auto localPlayer = mem::Read<LONGLONG>(mem::baseAddress + offsets::dwLocalPlayerPawn);
            if (localPlayer) {
                int32_t m_fFlags = mem::Read<int32_t>(localPlayer + offsets::fFlags);
                if (GetAsyncKeyState(VK_SPACE) && (m_fFlags & (1 << 0))) {
                    mem::Write<int>(mem::baseAddress + offsets::Force_Jump, 65537);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    mem::Write<int>(mem::baseAddress + offsets::Force_Jump, 256);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}


bool showmenu = false;

void loop() {
    uintptr_t localPlayer = mem::Read<uintptr_t>(mem::baseAddress + offsets::dwLocalPlayerPawn);
    view_matrix_t matrix = mem::Read<view_matrix_t>(mem::baseAddress + offsets::dwViewMatrix);



    Vector3 bestTargetHeadPos;
    float bestFov = FLT_MAX;
    uintptr_t bestEntityPawn = 0;



    for (int i = 1; i < 64; ++i) {
        Vector3 localPlayerPos = mem::Read<Vector3>(localPlayer + offsets::vecOrigin);
        Vector3 viewAngles = mem::Read<Vector3>(mem::baseAddress + offsets::dwViewAngles);
        Entity Entity;

        if (GetEntity(i, localPlayer, Entity)) {

            uintptr_t gameScene = mem::Read<uintptr_t>(Entity.entityPawn + offsets::m_pGameSceneNode);
            uintptr_t boneArray = mem::Read<uintptr_t>(gameScene + offsets::m_modelState + 0x80);


            Vector3 ScreenPosition = worldToScreen(matrix, Entity.position);

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();

            if (ScreenPosition.z > 0.001f) {
                float boxWidth = 0.0f;
                ImVec2 boxMin, boxMax;

                if (config::player_snaplines) {
                    drawList->AddLine(
                        ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y),
                        ImVec2(ScreenPosition.x, ScreenPosition.y),
                        IM_COL32(255, 255, 255, 255),
                        1.0f
                    );
                }

                if (config::player_box) {
                    Vector3 viewOffset = mem::Read<Vector3>(Entity.entityPawn + offsets::m_vecViewOffset);
                    Vector3 topPos = { Entity.position.x, Entity.position.y, Entity.position.z + viewOffset.z + 12 };
                    Vector3 bottomPos = { Entity.position.x, Entity.position.y, Entity.position.z - 6 };

                    Vector3 screenTop = worldToScreen(matrix, topPos);
                    Vector3 screenBottom = worldToScreen(matrix, bottomPos);

                    float boxHeight = fabs(screenBottom.y - screenTop.y);
                    boxWidth = boxHeight / 1.7f;

                    boxMin = ImVec2(screenTop.x - boxWidth / 2.0f, screenTop.y);
                    boxMax = ImVec2(screenTop.x + boxWidth / 2.0f, screenBottom.y);

                    drawList->AddRect(boxMin, boxMax, IM_COL32(255, 255, 255, 255), 0.5f, 0, 2.0f);
                }

                if (config::Player_bones) {
                    ImU32 boneColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); 
                    for (const auto& bonePair : boneConnections) {
                        Vector3 bone1 = mem::Read<Vector3>(boneArray + bonePair.bone1 * 32);
                        Vector3 bone2 = mem::Read<Vector3>(boneArray + bonePair.bone2 * 32);
                        Vector3 screenBone1 = worldToScreen(matrix, bone1);
                        Vector3 screenBone2 = worldToScreen(matrix, bone2);

                        if (screenBone1.z > 0.1f && screenBone2.z > 0.1f) {
                            drawList->AddLine(ImVec2(screenBone1.x, screenBone1.y), ImVec2(screenBone2.x, screenBone2.y), boneColor, 2.0f);
                        }
                    }
                }


                if (config::player_distance) {
                    Vector3 viewOffset = mem::Read<Vector3>(Entity.entityPawn + offsets::m_vecViewOffset);
                    Vector3 topPos = { Entity.position.x, Entity.position.y, Entity.position.z + viewOffset.z + 20 };
                    Vector3 bottomPos = { Entity.position.x, Entity.position.y, Entity.position.z - 6 };
                    Vector3 screenTop = worldToScreen(matrix, topPos);
                    Vector3 screenBottom = worldToScreen(matrix, bottomPos);

                    float distance = calculateDistance(localPlayerPos, Entity.position);

                    distance = distance / 100;
                   
                    char distanceText[32];
                    snprintf(distanceText, sizeof(distanceText), "%.2f m", distance);

               
                    ImVec2 textSize = ImGui::CalcTextSize(distanceText);
                    ImVec2 textPos = ImVec2(screenBottom.x - textSize.x / 2, screenBottom.y + 2);
                    

                    drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), distanceText);
                    
                }

         

                if (config::player_healthbar) {
                    int entityHealth = mem::Read<int>(Entity.entityPawn + offsets::m_iHealth);
                    DrawHealthbar(boxMin, boxMax, entityHealth);
                }

                if (config::Aimbot) {
                    uintptr_t gameScene = mem::Read<uintptr_t>(Entity.entityPawn + offsets::m_pGameSceneNode);
                    uintptr_t boneArray = mem::Read<uintptr_t>(gameScene + offsets::m_modelState + 0x80);

                    Vector3 headBone = mem::Read<Vector3>(boneArray + 6 * 32);
                    Vector3 headPos = headBone;
                    Vector3 screenPos = worldToScreen(matrix, headPos);

                    Vector3 angleToHead = CalcAngle(localPlayerPos, headPos);
                    float fov = CalculateFOV(viewAngles, angleToHead);

                    float screenCenterX = static_cast<float>(ImGui::GetIO().DisplaySize.x) / 2.0f;
                    float screenCenterY = static_cast<float>(ImGui::GetIO().DisplaySize.y) / 2.0f;
                    float fovToCenter = sqrt(pow(screenPos.x - screenCenterX, 2) + pow(screenPos.y - screenCenterY, 2));

                    if (fovToCenter < config::FOV * 10 && fovToCenter < bestFov) {
                        bestFov = fovToCenter;
                        bestEntityPawn = Entity.entityPawn;
                        bestTargetHeadPos = headPos;
                    }
                }
            }
        }
    }

    if (GetAsyncKeyState(VK_SHIFT) & 0x8000 && config::Aimbot && bestEntityPawn != 0) {
        Vector3 screenPos = worldToScreen(matrix, bestTargetHeadPos);
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);

        Vector3 targetScreenPos = { screenPos.x, screenPos.y, 0.0f };
        float smoothFactor = static_cast<float>(config::Smoothing);

        float smoothing = 1.0f / (smoothFactor / 10.0f);

        if (smoothFactor > 0) {
            targetScreenPos = Lerp(Vector3{ static_cast<float>(currentMousePos.x), static_cast<float>(currentMousePos.y), 0.0f }, targetScreenPos, smoothing);
        }

        int deltaX = static_cast<int>(targetScreenPos.x - currentMousePos.x);
        int deltaY = static_cast<int>(targetScreenPos.y - currentMousePos.y);

        mouse_event(MOUSEEVENTF_MOVE, deltaX, deltaY, 0, 0);
    }

    if (GetAsyncKeyState(VK_INSERT))
    {
        showmenu = !showmenu;
        Sleep(500);
    }
}
        
        


bool init = false;
int selectedTab = 0;

const char* tabNames[] = { "Combat", "Visuals", "Mics" };

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!init)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
        {
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            window = sd.OutputWindow;

            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();

            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            InitImGui();

            init = true;
        }
        else
            return oPresent(pSwapChain, SyncInterval, Flags);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(10, 5);
    style.ScrollbarSize = 16;
    style.WindowPadding = ImVec2(10, 10);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    colors[ImGuiCol_Border] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.1f, 0.1f, 0.1f, 0.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.2f, 0.2f, 0.2f, 0.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.3f, 0.3f, 0.3f, 0.0f);
    colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.3f, 0.3f, 0.3f, 0.6f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.5f, 0.5f, 0.8f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);

    ImGui::SetWindowSize(ImVec2(1000, 1000));

    ImGui::SetNextWindowSize(ImVec2(400, 300));

    if (showmenu)
    {


        ImGui::Begin("Internal");

        ImGui::BeginChild("Tabs", ImVec2(120, 0), true);
        for (int i = 0; i < IM_ARRAYSIZE(tabNames); i++)
        {
            ImGui::PushID(i);

            ImGui::Button("", ImVec2(-1, 30));

            if (selectedTab == i)
            {
                ImGui::SetItemDefaultFocus();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), tabNames[i]);
            }
            else
            {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
                ImGui::Text(tabNames[i]);
            }

            if (ImGui::IsItemClicked())
            {
                selectedTab = i;
            }
            ImGui::PopID();
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("Content", ImVec2(0, 0), true);
        switch (selectedTab)
        {
        case 0:
            ImGui::Text("Combat");
            ImGui::Checkbox("Aimbot", &config::Aimbot);
            ImGui::Text("FOV: %.2f", config::FOV);
            ImGui::SliderFloat("##fov", &config::FOV, 0.0f, 50.0f, "");
            ImGui::Text("Smoothing: %.2f", config::Smoothing);
            ImGui::SliderFloat("##smoothing", &config::Smoothing, 0.0f, 50.0f, "");


            ImGui::Checkbox("Triggerbot", &config::trigger_bot);
            ImGui::PopStyleVar(2);

            if (config::trigger_bot) {
                ImGui::Text("Delay: %d ms", config::triggerdelay);
                ImGui::SliderInt("##Triggerbot Delay", &config::triggerdelay, 0, 1000, "");

            }

            break;
        case 1:
            ImGui::Text("Visuals");

            ImGui::Checkbox("Player Box", &config::player_box);
            ImGui::Checkbox("player Skeleton", &config::Player_bones);
            ImGui::Checkbox("player distance", &config::player_distance);
            ImGui::Checkbox("Player Health Bar", &config::player_healthbar);
            ImGui::Checkbox("Player Snaplines", &config::player_snaplines);

            break;
        case 2:
            ImGui::Text("Mics");
            ImGui::Checkbox("Bunny Hop", &config::bunny_hop);

            break;
        }
        ImGui::EndChild();

        ImGui::End();
    }

    loop();

    ImGui::Render();
    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    AllocConsole();
    FILE* Dummy;
    freopen_s(&Dummy, "CONOUT$", "w", stdout);
    freopen_s(&Dummy, "CONIN$", "r", stdin);

    std::thread triggerbot(Trigger_bot);
    std::thread bhop(bunny_hop);

    cout << "Game Hooked: " << hex << mem::baseAddress << endl;
    bool init_hook = false;
    do
    {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
        {
            kiero::bind(8, (void**)&oPresent, hkPresent);
            init_hook = true;
        }
    } while (!init_hook);

    triggerbot.join();
    bhop.join();

    return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        kiero::shutdown();
        break;
    }
    return TRUE;
}