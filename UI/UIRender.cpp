#include "UIRender.h"

#include <functional>
#include <ranges>

#include "UIState.h"

#include "..//OS-ImGui.h"

#include "unordered_map"

#include <utility>
struct funcInfo {
    std::function<void()> func;
    bool repeat;
};

std::unordered_map<std::string, int> hotkeyIds; // 标识符到键的映射
std::unordered_map<int, funcInfo> hotkeyBindings; // 键到函数信息的映射
std::unordered_map<std::string,std::function<void()>> draw;


// 获取 std::function 唯一标识（hash）
size_t getFuncHash(const std::function<void()>& func) {
    return reinterpret_cast<size_t>(func.target<void()>());
}
//实际调用绑定hotkey的func
void checkHotkeys() {
    for (auto& [key, func] : hotkeyBindings) {
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(key), func.repeat)) {
            if (func.func) func.func(); // 触发
        }
    }
}
//在UI线程添加sth
void runOnUI() {
    for (auto &func: draw | std::views::values) {
        if (func)func();
    }
}

//绘制BoneESP
void drawBone() {
    if (ImGui::BeginTabItem("BoneESP")) {
        ImGui::Checkbox("Enable",&UI::Bone::enable);
        ImGui::ColorEdit3("Color",UI::Bone::color);
        ImGui::EndTabItem();
    }
}
static const char* pre_hotkey_name[]={"LAlt","Caps","LCTRL"};

void drawAimbot() {

    auto drawSelectHotkey=[]{
        // 计算当前选中的索引
        int currentIndex = 0;
        for (int i = 0; i < 3; i++) {
            if (UI::Aimbot::pre_hotkey[i] == UI::Aimbot::select_hotkey) {
                currentIndex = i;
                break;
            }
        }

        if (ImGui::BeginCombo("Hotkey", pre_hotkey_name[currentIndex])) {
            for (int i = 0; i < 3; i++) {
                bool isSelected = (currentIndex == i);
                if (ImGui::Selectable(pre_hotkey_name[i], isSelected)) {
                    UI::Aimbot::select_hotkey = UI::Aimbot::pre_hotkey[i]; // 更新当前选择
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    };

    if (ImGui::BeginTabItem("Aimbot")) {
        ImGui::Checkbox("Enable",&UI::Aimbot::enable);

        drawSelectHotkey();
        // 如果热键改变了，更新绑定
        static int lastSelectedHotkey = UI::Aimbot::select_hotkey;
        if (lastSelectedHotkey != UI::Aimbot::select_hotkey) {
            updateHotkey("aimbot_action", UI::Aimbot::select_hotkey);
            lastSelectedHotkey = UI::Aimbot::select_hotkey;
        }
        ImGui::SliderFloat("Radius",&UI::Aimbot::circle_radius,1,360,"%.1f");

        ImGui::ColorEdit3("Color",UI::Aimbot::color);
        ImGui::EndTabItem();
    }
}
void drawFeature(){
    if (ImGui::BeginTabBar("Feature")) {
        drawBone();
        drawAimbot();
        ImGui::EndTabBar();
    }
}
void drawUI() {
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f),ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 180.0f),ImGuiCond_Once);
    if (UI::showMenu) {
        ImGui::Begin("coldFish[INSERT]");

        drawFeature();
        runOnUI();
        ImGui::End();
    }
    checkHotkeys();
}


void bindHotkey(const std::string& id,int newKey, std::function<void()> function, bool repeat) {
    if (hotkeyIds.find(id) != hotkeyIds.end()) {
        int oldKey = hotkeyIds[id];
        hotkeyBindings.erase(oldKey);
    }

    // 设置新绑定
    hotkeyBindings[newKey] = funcInfo{function, repeat};
    hotkeyIds[id] = newKey;
}
void updateHotkey(const std::string& id, int newKey) {
    if (hotkeyIds.find(id) != hotkeyIds.end() && hotkeyBindings.find(hotkeyIds[id]) != hotkeyBindings.end()) {
        // 保存函数信息
        funcInfo info = hotkeyBindings[hotkeyIds[id]];

        // 移除旧绑定
        hotkeyBindings.erase(hotkeyIds[id]);

        // 创建新绑定
        hotkeyBindings[newKey] = info;
        hotkeyIds[id] = newKey;
    }
}

void addUIRunFunc(const std::string &funcName, std::function<void()> func) {
    draw[funcName] = std::move(func);
}

