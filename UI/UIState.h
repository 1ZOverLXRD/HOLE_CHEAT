#pragma once

#ifndef HOLECHEAT_UISTATE_H
#define HOLECHEAT_UISTATE_H

namespace UI{
    inline bool showMenu=true;
    inline float thickness=0.618;
    namespace Bone {
        inline bool enable=false;
        inline float thickness=0.618;
        inline float color[3]={0.0f,1.0f,0.0f};
    }
    namespace Aimbot {

        inline bool enable=false;
        //LAlt,Caps,LCtrl
        inline int pre_hotkey[]={529,595,527};
        //当前hotkey
        inline int select_hotkey=595;

        inline float circle_radius=100.0f;
        //线条颜色
        inline float color[3]={1.0f,0.0f,0.0f};
        //TODO 平滑度
        inline float smooth=0.2f;
    }
}

#endif //HOLECHEAT_UISTATE_H