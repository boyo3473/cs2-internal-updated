#include "mem.h"
#include "offsets.h"
#include <cstdint>
#include "Vector3.h"

struct Entity {
    uintptr_t entityPawn;
    int health;
    int teamNum;
    Vector3 position;
    int id;

    Entity() : entityPawn(0), health(0), teamNum(0), position{ 0, 0, 0 }, id(0) {}
};


bool GetEntity(int playerIndex, uintptr_t localPlayer, Entity& Entity) {
    uintptr_t entityList = mem::Read<uintptr_t>(mem::baseAddress + offsets::dwEntityList);

    uintptr_t listEntry1 = mem::Read<uintptr_t>(entityList + ((8 * (playerIndex & 0x7FFF) >> 9) + 16));
    if (!listEntry1) return false;

    uintptr_t playerController = mem::Read<uintptr_t>(listEntry1 + 120 * (playerIndex & 0x1FF));
    if (!playerController) return false;

    uint32_t PlayerPawn1 = mem::Read<uint32_t>(playerController + offsets::m_hPlayerPawn);
    if (!PlayerPawn1) return false;

    uintptr_t listEntry2 = mem::Read<uintptr_t>(entityList + 0x8 * ((PlayerPawn1 & 0x1FFF) >> 9) + 16);
    if (!listEntry2) return false;

    uintptr_t entityPawn = mem::Read<uintptr_t>(listEntry2 + 120 * (PlayerPawn1 & 0x1FF));
    if (!entityPawn) return false;

    if (entityPawn == localPlayer) return false;  


    Entity.entityPawn = entityPawn;
    Entity.health = mem::Read<int>(entityPawn + offsets::m_iHealth);
    if (Entity.health <= 0) return false;  

    Entity.teamNum = mem::Read<int>(entityPawn + offsets::m_iTeamNum);
    Entity.position = mem::Read<Vector3>(entityPawn + offsets::vecOrigin);
    Entity.id = playerIndex;

    return true;
}
