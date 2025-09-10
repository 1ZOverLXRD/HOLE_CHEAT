#include "AcotrMannager.h"


#include "..//pch.h"



bool AcotrManner::update(SDK::UWorld *uWorld) noexcept {
    this->actors_.clear();
    //check some ptr
    checkptr(uWorld);
    auto persistentLevel=uWorld->PersistentLevel;
    checkptr(persistentLevel);
    auto owingGame=uWorld->OwningGameInstance;
    checkptr(owingGame);
    if (!owingGame->LocalPlayers)return false;
    for (auto i=0;i<persistentLevel->Actors.Num();i++) {
        auto actor=persistentLevel->Actors[i];
        if (!actor)continue;//skip nullptr
        this->actors_.push_back(actor);
    }
    if (owingGame->LocalPlayers.Num()<=0)return false;
    auto localP=owingGame->LocalPlayers[0];
    if (!localP->PlayerController)return false;
    this->localPlyer=localP;
    //printf("Update:LocalPlayerController ptr:%p",localP);
    return true;
}
//TODO 过滤Actor
std::vector<SDK::AActor*> AcotrManner::fliterActor() {
    std::unordered_map<std::string, int> classCount;
    std::vector<SDK::AActor*> filteredActors;
    filteredActors.reserve(this->actors_.size());

    // 先统计每个类的数量，并初步过滤指定类名
    for (auto actor : this->actors_) {
        if (!actor || !actor->Class) continue;

        std::string className = actor->Class->GetName();

        // 初步过滤不需要的类
        if (className.contains("StaticMesh") || className.contains("CoverCube_Black_C") ||
            className.contains("BPLight_LongLamp_A_C") || className.contains("DC_Home_dirt_A_C") ||
            className.contains("Light") || className.contains("Bullet") || className.contains("BP_MiniHole") ||
            className.contains("writing_desk_SP_C") || className.contains("CeilingTile_A_C") ||
            className.contains("display_SM_C") || className.contains("LCBP_EnemySpawner_C") ||
            className.contains("SteelRack_SP_C") || className.contains("BP_Peg")) {
            continue;
        }

        classCount[className]++;
    }

    // 遍历 actors_，只保留数量 <= 50 的类
    for (auto actor : this->actors_) {
        if (!actor || !actor->Class) continue;

        std::string className = actor->Class->GetName();

        // 再次过滤指定类名
        if (className.contains("StaticMesh") || className.contains("CoverCube_Black_C") ||
            className.contains("BPLight_LongLamp_A_C") || className.contains("DC_Home_dirt_A_C") ||
            className.contains("Light") || className.contains("Bullet") || className.contains("BP_MiniHole") ||
            className.contains("writing_desk_SP_C") || className.contains("CeilingTile_A_C") ||
            className.contains("display_SM_C") || className.contains("LCBP_EnemySpawner_C") ||
            className.contains("SteelRack_SP_C") || className.contains("BP_Peg")) {
            continue;
        }

        // 只保留数量 <= 50 的类
        if (classCount[className] <= 50) {
            filteredActors.push_back(actor);
        }
    }

    return filteredActors;
}

auto sp = std::make_unique<FVector2D>();

std::vector<SDK::ABP_NPCBase_C *> AcotrManner::fliteNPC() {
    //ABP_NPCBase_C
    std::vector<SDK::ABP_NPCBase_C*> filteredActors;
    filteredActors.reserve(this->actors_.size());

    // 先统计每个类的数量，并初步过滤指定类名
    for (auto actor : this->actors_) {
        if (!actor || !actor->Class) continue;
        if (actor->IsA(ABP_NPCBase_C::StaticClass())) {
            auto npc=reinterpret_cast<ABP_NPCBase_C*>(actor);
            filteredActors.push_back(npc);
        }
    }
    return filteredActors;
}

void AcotrManner::printTOP5() {
    std::unordered_map<std::string, int> classCount;

    //忽略:StaticMesh
    for (auto actor:this->actors_) {
        auto className=actor->Class->GetName();
        //通过类名过滤

        if (className.contains("StaticMesh")||className.contains("CoverCube_Black_C")||className.contains("BPLight_LongLamp_A_C")||
            className.contains("DC_Home_dirt_A_C")||className.contains("Light"))continue;
        classCount[className]++;
    }
    std::vector<std::pair<std::string, int>> vec(classCount.begin(), classCount.end());

    // 按出现次数排序，降序
    std::sort(vec.begin(), vec.end(), [](const auto &a, const auto &b) {
        return a.second > b.second;
    });
    std::cout << "Top 5 Actor Classes:\n";
    for (int i = 0; i < 5 && i < vec.size(); i++) {
        std::cout << vec[i].first << " : " << vec[i].second << "\n";
    }
}

void AcotrManner::drawAllActor() {

    for (auto actor:this->fliteNPC()) {
        if (!actor->RootComponent)continue;
        auto pos=actor->RootComponent->RelativeLocation;

        if (this->localPlyer->PlayerController->ProjectWorldLocationToScreen(pos,sp.get(),false)){
            if (sp->X<=0||sp->Y<=0)continue;
            Gui.Text(actor->Class->GetName(),{(float)sp->X,(float)sp->Y},ImColor(255,255,255,255));
        }
    }
}
