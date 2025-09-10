#pragma once

#ifndef HOLECHEAT_UIRENDER_H
#define HOLECHEAT_UIRENDER_H

#include <functional>
#include <string>

void drawUI();


//绑定hotkey
void bindHotkey(const std::string& id, int newKey, std::function<void()> function, bool repeat = false);
void updateHotkey(const std::string& id, int newKey);
//添加UI后置添加方法
void addUIRunFunc(const std::string &funcName,std::function<void()> func);

#endif //HOLECHEAT_UIRENDER_H