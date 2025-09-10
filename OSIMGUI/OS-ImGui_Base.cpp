#include "OS-ImGui_Base.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui_Base.cpp
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.1
* @date			: 2024/4/4 13:59
****************************************************/

namespace OSImGui
{
    DWORD OSImGui_Base::GetPIDByProcessName(const std::string& processName)
    {
        DWORD pid = 0;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
            return 0;

        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(snapshot, &pe)) {
            do {
                if (!_stricmp(pe.szExeFile, processName.c_str())) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &pe));
        }
        CloseHandle(snapshot);
        return pid;
    }
    HWND OSImGui_Base::FindWindowByPID(DWORD pid)
    {
        HWND hWnd = nullptr;
        struct EnumData {
            DWORD pid;
            HWND hWnd;
        } data{ pid, nullptr };

        EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL {
            EnumData* pData = reinterpret_cast<EnumData*>(lParam);
            DWORD winPid;
            GetWindowThreadProcessId(hWnd, &winPid);
            if (winPid == pData->pid && IsWindowVisible(hWnd)) {
                pData->hWnd = hWnd;
                return FALSE; // ÕÒµ½ÁË¾ÍÍ£Ö¹
            }
            return TRUE;
            }, reinterpret_cast<LPARAM>(&data));

        return data.hWnd;
    }
    bool OSImGui_Base::InitImGui(ID3D11Device* device, ID3D11DeviceContext* device_context)
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();
        io.LogFilename = nullptr;

        if (!ImGui_ImplWin32_Init(Window.hWnd))
            throw OSException("ImGui_ImplWin32_Init() call failed.");
        if (!ImGui_ImplDX11_Init(device, device_context))
            throw OSException("ImGui_ImplDX11_Init() call failed.");

        return true;
    }

    void OSImGui_Base::CleanImGui()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if(g_Device.g_pd3dDevice)
            g_Device.CleanupDeviceD3D();

        // Only destroy window in external mode.
        if (Window.hWnd && !Window.Name.empty())
        {
            DestroyWindow(Window.hWnd);
            UnregisterClassA(Window.ClassName.c_str(), Window.hInstance);
        }
    }

    std::wstring OSImGui_Base::StringToWstring(std::string& str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str);
    }

}