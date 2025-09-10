#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>


namespace SDK {
    class UWorld;
    class AActor;
    class ULocalPlayer;
    class ABP_NPCBase_C;
}

#define checkptr(arg) if(!arg){printf("%s is nullptr\n",#arg);return false;}

class AcotrManner {
private:
    std::vector<SDK::AActor*> actors_;
    SDK::ULocalPlayer* localPlyer;
public:
    AcotrManner()=default;
    //Actor
    std::vector<SDK::AActor*> getActors(){return actors_;}
    //Actor大小
    int getActorsNum(){return actors_.size();}
    //更新Actor
    bool update(SDK::UWorld* uWorld) noexcept;
    //TODO 过滤Actor
    std::vector<SDK::AActor*> fliterActor();

    //LocalPlayer
    SDK::ULocalPlayer* getLocalPlayer(){return localPlyer;}

    std::vector<SDK::ABP_NPCBase_C*> fliteNPC();

    //获取Actor中的前5个最多的
    void printTOP5();
    //绘制所有Actor
    void drawAllActor();
};