#include "coldFish.h"

#include "OSIMGUI/OS-ImGui.h"
#include "pch.h"

#include "ActorMannager/AcotrMannager.h"
#include "Aimboter/Aimboter.h"
#include "BoneESP/BoneHelper.h"
#include "UI/UIRender.h"
#include "UI/UIState.h"

AcotrManner actorMannager;
ULocalPlayer* localPlayer;
BoneHelper boneHelper;
Aimbot aimbot=Aimbot(&boneHelper);

#define INVOKE_IF(cond, func, ...) do { if (cond) func(__VA_ARGS__); } while(0)
#define BIND_HOTKEY_CHANGE_STATUS(ID,key,STATUS) bindHotkey(ID,key,[]{STATUS=!STATUS;});

//检测UI状态并且运行
void checkUIState() {
    INVOKE_IF(UI::Bone::enable,boneHelper.drawBones,actorMannager.fliteNPC(),localPlayer->PlayerController);
    INVOKE_IF(UI::Aimbot::enable,aimbot.enable,actorMannager.fliteNPC(),localPlayer->PlayerController,false);
}
//更新数据并且检测UI状态再运行
bool upDate() {
    if (const auto uWorld=UWorld::GetWorld()) {
        if (actorMannager.update(uWorld)) {
            localPlayer=actorMannager.getLocalPlayer();
            checkUIState();
            return true;
        }
    }
    return false;
}

//不会改变的key
void registerStaticHotkeys() {
    BIND_HOTKEY_CHANGE_STATUS("toggle_menu",ImGuiKey_Insert,UI::showMenu);
    BIND_HOTKEY_CHANGE_STATUS("toggle_bone", ImGuiKey_F1, UI::Bone::enable);
    BIND_HOTKEY_CHANGE_STATUS("toggle_aimbot", ImGuiKey_F2, UI::Aimbot::enable);

}

// 需要动态更新的热键
void registerDynamicHotkeys() {
    static int lastHotkey = -1;
    if (lastHotkey != UI::Aimbot::select_hotkey) {
        bindHotkey("aimbot_action", UI::Aimbot::select_hotkey, [] {INVOKE_IF(UI::Aimbot::enable,aimbot.enable,actorMannager.fliteNPC(), localPlayer->PlayerController, true);}, true);
        lastHotkey = UI::Aimbot::select_hotkey;
    }
}
void registerHotkey(){
    // 只在首次调用时注册静态热键
    static bool firstRun = true;
    if (firstRun) {
        registerStaticHotkeys();
        firstRun = false;
    }
    // 每帧检查并更新动态热键
    registerDynamicHotkeys();
}
void mainLoop() {
    upDate();
    registerHotkey();
    drawUI();
}
