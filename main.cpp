#include "pch.h"
#include "coldFish.h"

void openConsole(HMODULE hmodule) {
    AllocConsole();
    FILE* pFile;
    freopen_s(&pFile, "CONIN$", "r", stdin);
    freopen_s(&pFile, "CONOUT$", "w", stdout);
    freopen_s(&pFile, "CONOUT$", "w", stderr);
    SetConsoleTitleA("ColdFish");
    Gui.Start(hmodule,mainLoop,OSImGui::DX12);
    printf("Inject\n");

}
BOOL APIENTRY DllMain(HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(0,0,(LPTHREAD_START_ROUTINE)openConsole,(void*)hModule,0,0);
    }
    return TRUE;
}