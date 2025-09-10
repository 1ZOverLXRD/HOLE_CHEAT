#include <cstdint>
#include <utility>
#include <vector>

struct Vector3_ {
    double X,Y,Z;
};
struct Vector2D_ {
    double x,y;
};
struct BoneStruct {
    char* boneName;
    Vector3_ worldPos;
    Vector2D_ screenPos;
};
namespace SDK {
    class UWorld;
    class AActor;
    class ULocalPlayer;
    class APlayerController;
    class ABP_NPCBase_C;
    class USkeletalMeshComponent;
}
class BoneHelper {
private:
    void DrawBoneByName(SDK::USkeletalMeshComponent* Mesh, const char* BoneNameA, const char* BoneNameB);
public:
    BoneHelper()=default;
    void printBones(SDK::ABP_NPCBase_C* npc);
    bool getBone(const char *bonename, SDK::ABP_NPCBase_C* npc, SDK::APlayerController* localPlayerCTLER, BoneStruct& OUT_boneStruct);
    void drawBones(SDK::ABP_NPCBase_C* npc,SDK::APlayerController* localPlayerCTLER);
    void drawBones(std::vector<SDK::ABP_NPCBase_C*> npcs,SDK::APlayerController* localPlayerCTLER);
};