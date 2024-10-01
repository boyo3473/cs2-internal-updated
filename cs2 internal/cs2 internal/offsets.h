#include <cstddef>
namespace offsets {
    // Camera offsets
    inline constexpr uintptr_t cameraX = 0x19D1E08 + 4;
    inline constexpr uintptr_t cameraY = 0x19D1E08;

    // Buttons.hpp offsets
    inline constexpr std::ptrdiff_t Force_Jump = 0x17BA530;

    // offsets.hpp
    inline constexpr std::ptrdiff_t dwEntityList = 0x1954568;
    inline constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x17C17F0;
    inline constexpr std::ptrdiff_t dwViewAngles = 0x19C5958;
    inline constexpr std::ptrdiff_t m_hPlayerPawn = 0x7EC; // CHandle<C_CSPlayerPawn>
    inline constexpr std::ptrdiff_t dwViewMatrix = 0x19B64F0;

    // client.dll offsets
    inline constexpr std::ptrdiff_t m_iIDEntIndex = 0x13A8;
    inline constexpr std::ptrdiff_t m_iTeamNum = 0x3C3;
    inline constexpr std::ptrdiff_t m_iHealth = 0x324;
    inline constexpr std::ptrdiff_t fFlags = 0x3CC;
    inline constexpr std::ptrdiff_t vecOrigin = 0x1274;
    inline constexpr std::ptrdiff_t m_flFlashDuration = 0x135C;

    inline constexpr std::ptrdiff_t m_entitySpottedState = 0x2288; // EntitySpottedState_t
    inline constexpr std::ptrdiff_t m_bSpottedByMask = 0xC;

    inline constexpr std::ptrdiff_t m_iszPlayerName = 0x630; // char[128]
    inline constexpr std::ptrdiff_t m_iBoneIndex = 0xF00; // int32
    inline constexpr std::ptrdiff_t m_vecViewOffset = 0xC50; // CNetworkViewOffsetVector
    inline constexpr std::ptrdiff_t m_pGameSceneNode = 0x308; // CGameSceneNode*
    inline constexpr std::ptrdiff_t m_modelState = 0x170; // CModelState


    inline constexpr std::ptrdiff_t m_iAccount = 0x40; // int32
    inline constexpr std::ptrdiff_t m_pInGameMoneyServices = 0x6F0; // CCSPlayerController_InGameMoneyServices*

    inline constexpr std::ptrdiff_t m_aimPunchAngle = 0x14CC; // QAngle
    inline constexpr std::ptrdiff_t m_iShotsFired = 0x22B4; // int32

    inline constexpr std::ptrdiff_t m_firePositions = 0xD38; // Vector[64]

    inline constexpr std::ptrdiff_t m_pCameraServices = 0x1130; // CPlayer_CameraServices*
    inline constexpr std::ptrdiff_t m_iFOV = 0x210; // uint32




    constexpr std::ptrdiff_t dwCSGOInput = 0x19BFBD8;

    constexpr std::ptrdiff_t m_pObserverServices = 0x1110;
    constexpr std::ptrdiff_t m_iObserverMode = 0x40;

    constexpr std::ptrdiff_t m_flVisibilityStrength = 0x548;
}
