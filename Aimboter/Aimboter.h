#pragma once

#include <vector>
namespace SDK {
    class UWorld;
    class AActor;
    class ULocalPlayer;
    class APlayerController;
    class ABP_NPCBase_C;
    class USkeletalMeshComponent;
}
class BoneHelper;
class Aimbot {
private:
    SDK::ABP_NPCBase_C* getClosestEnemy(const std::vector<SDK::ABP_NPCBase_C*> &enemy,SDK::APlayerController* localPlayer,int screenWidth,int screenHeight,float radius) const;
    SDK::ABP_NPCBase_C* getClosestEnemy(const std::vector<SDK::ABP_NPCBase_C*> &enemy,SDK::APlayerController* localPlayer,float radius) const;
    BoneHelper* boneHelper;

    static double* calculateAngle(const double* enemy, const SDK::APlayerController* localPlayer);

    static void writeAngle(SDK::APlayerController* localPlayer, const double* angle);
public:
    Aimbot()=default;

    Aimbot(BoneHelper* bh){this->boneHelper=bh;};

    void enable(const std::vector<SDK::ABP_NPCBase_C*> &enemy,SDK::APlayerController* localPlayer,bool hotkeyState) const;
};