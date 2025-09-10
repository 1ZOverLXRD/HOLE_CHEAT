#include "BoneHelper.h"

#include "..//pch.h"
#include <memory>

#include "../UI/UIState.h"

static const std::vector<std::pair<const char*, const char*>> SkeletonLinks = {
    {"spine_01", "spine_02"},
    {"spine_02", "spine_03"},
    {"spine_03", "Head"},
    {"spine_02", "upperarm_l"},
    {"upperarm_l", "lowerarm_l"},
    {"lowerarm_l", "hand_l"},
    {"spine_02", "upperarm_r"},
    {"upperarm_r", "lowerarm_r"},
    {"lowerarm_r", "hand_r"},
    {"pelvis", "thigh_l"},
    {"thigh_l", "calf_l"},
    {"calf_l", "foot_l"},
    {"pelvis", "thigh_r"},
    {"thigh_r", "calf_r"},
    {"calf_r", "foot_r"},
};

void BoneHelper::printBones(SDK::ABP_NPCBase_C* npc) {
    USkeletalMeshComponent*  mesh=npc->Mesh;
    if (!mesh||!npc||!mesh->SkeletalMesh){printf("!mesh||!npc||!mesh->SkeletalMesh\n");return;};
    for (int i=0;i<mesh->GetNumBones();i++) {
        auto boneName=mesh->GetBoneName(i);
        printf("%s-%d:%s\n",npc->GetName().c_str(),i,boneName.ToString().c_str());
    }
}

bool BoneHelper::getBone(const char *bonename, SDK::ABP_NPCBase_C* npc, SDK::APlayerController* localPlayerCTLER, BoneStruct& OUT_boneStruct) {
    USkeletalMeshComponent*  mesh=npc->Mesh;
    if (!mesh||!npc||!mesh->SkeletalMesh||!localPlayerCTLER)return false;
    std::unordered_map<std::string, int> boneMap;
    for (int i = 0; i < mesh->GetNumBones(); i++) {
        auto boneName_FNAME = mesh->GetBoneName(i);
        if (boneName_FNAME.ToString()==bonename) {
            auto pos=mesh->GetBoneTransform(boneName_FNAME, ERelativeTransformSpace::RTS_World).Translation;
            auto screen = std::make_unique<FVector2D>();
            if (localPlayerCTLER->ProjectWorldLocationToScreen(pos,screen.get(),false)) {
                OUT_boneStruct=BoneStruct{(char*)bonename,{pos.X,pos.Y,pos.Z},{screen->X,screen->Y}};
                return true;
            }
        }
    }
    return false;
}


void BoneHelper::drawBones(SDK::ABP_NPCBase_C *npc,APlayerController* localPlayerCTLER) {
    USkeletalMeshComponent*  mesh=npc->Mesh;
    if (!mesh||!npc||!mesh->SkeletalMesh)return;

    //// 先收集骨骼名字映射
    std::unordered_map<std::string, int> boneMap;
    for (int i = 0; i < mesh->GetNumBones(); i++) {
        auto boneName = mesh->GetBoneName(i);
        boneMap[boneName.ToString()] = i;
    }
    // 遍历要连的骨骼
    for (const auto& link : SkeletonLinks) {
        auto itA = boneMap.find(link.first);
        auto itB = boneMap.find(link.second);

        if (itA == boneMap.end() || itB == boneMap.end()) continue;

        auto boneNameA = mesh->GetBoneName(itA->second);
        auto boneNameB = mesh->GetBoneName(itB->second);

        FTransform boneA = mesh->GetBoneTransform(boneNameA, ERelativeTransformSpace::RTS_World);
        FTransform boneB = mesh->GetBoneTransform(boneNameB, ERelativeTransformSpace::RTS_World);

        FVector posA = boneA.Translation;
        FVector posB = boneB.Translation;

        auto screenA = std::make_unique<FVector2D>();
        auto screenB = std::make_unique<FVector2D>();

        if (localPlayerCTLER->ProjectWorldLocationToScreen(posA, screenA.get(), false) &&
            localPlayerCTLER->ProjectWorldLocationToScreen(posB, screenB.get(), false)) {
            Gui.Line({(float)screenA->X, (float)screenA->Y}, {(float)screenB->X, (float)screenB->Y}, ImColor(UI::Bone::color[0],UI::Bone::color[1],UI::Bone::color[2]), 0.618f);
            }
    }
}

void BoneHelper::drawBones(std::vector<SDK::ABP_NPCBase_C *> npcs, SDK::APlayerController *localPlayerCTLER) {
    for (auto npc : npcs) {
        drawBones(npc, localPlayerCTLER); // 调用原来的单 NPC 版本
    }
}
