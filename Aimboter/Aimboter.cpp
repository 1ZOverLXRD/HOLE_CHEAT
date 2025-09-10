#include "Aimboter.h"

#include "..//pch.h"
#include "../BoneESP/BoneHelper.h"
#include "../UI/UIState.h"

#define PI 3.14159265358979323846

SDK::ABP_NPCBase_C * Aimbot::getClosestEnemy(const std::vector<SDK::ABP_NPCBase_C *> &enemys,SDK::APlayerController *localPlayer, int screenWidth, int screenHeight,float radius) const {
    SDK::ABP_NPCBase_C* closestEnemy = nullptr;
    BoneStruct boneStruct;
    FVector2D screenCenter(screenWidth / 2.0f, screenHeight / 2.0f);
    float closestDist = FLT_MAX;

    for (auto& enemy : enemys) {
        if (boneHelper->getBone("Head", enemy, localPlayer, boneStruct)) {
            auto bonePos = boneStruct.screenPos;

            // 计算到屏幕中心的 2D 距离
            float dx = bonePos.x - screenCenter.X;
            float dy = bonePos.y - screenCenter.Y;
            float dist = sqrtf(dx * dx + dy * dy);

            // 只考虑在 radius 内的敌人
            if (dist <= radius && dist < closestDist) {
                closestDist = dist;
                closestEnemy = enemy;
            }
        }
    }
    return closestEnemy;
}

SDK::ABP_NPCBase_C * Aimbot::getClosestEnemy(const std::vector<SDK::ABP_NPCBase_C *> &enemys,SDK::APlayerController *localPlayer,float radius) const {
    return getClosestEnemy(enemys,localPlayer,Gui.Window.Size.x,Gui.Window.Size.y,radius);
}

double* Aimbot::calculateAngle(const double* enemy, const SDK::APlayerController *localPlayer) {
    auto enemyPos=FVector(enemy[0],enemy[1],enemy[2]);
    auto dir=  enemyPos- localPlayer->PlayerCameraManager->ViewTarget.POV.Location;
    dir.Normalize();
    double yaw=atan2(dir.Y, dir.X)*(180.0/PI);
    double pitch=atan2(dir.Z, sqrt(dir.X * dir.X + dir.Y * dir.Y)) * (180 / PI);
    double angle[]={yaw,pitch};
    return angle;
}

void Aimbot::writeAngle(SDK::APlayerController *localPlayer, const double *angle) {
    localPlayer->ControlRotation.Yaw = angle[0];
    localPlayer->ControlRotation.Pitch = angle[1];
}

void Aimbot::enable(const std::vector<SDK::ABP_NPCBase_C *> &enemy,SDK::APlayerController* localPlayer,bool hotkeyState) const {
    Gui.Circle({Gui.Window.Size.x/2,Gui.Window.Size.y/2},UI::Aimbot::circle_radius,ImColor(UI::Aimbot::color[0],UI::Aimbot::color[1],UI::Aimbot::color[2]),0.618);

    if (!hotkeyState||enemy.size() <=0 || !localPlayer)return;
    auto closestEnemy=getClosestEnemy(enemy,localPlayer,UI::Aimbot::circle_radius);
    if (!closestEnemy)return;
    auto sp=std::make_unique<FVector2D>();
    auto pos=closestEnemy->RootComponent->RelativeLocation;
    BoneStruct boneStruct;
    if (localPlayer->ProjectWorldLocationToScreen(pos,sp.get(),false)) {
        if (boneHelper->getBone("Head",closestEnemy,localPlayer,boneStruct)) {
            double pos_[3]={boneStruct.worldPos.X,boneStruct.worldPos.Y,boneStruct.worldPos.Z};
            double* angle=this->calculateAngle(pos_,localPlayer);
            writeAngle(localPlayer,angle);
        }
    }
}
