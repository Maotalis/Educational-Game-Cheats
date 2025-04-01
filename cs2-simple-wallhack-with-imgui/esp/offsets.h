#pragma once
#include <cstddef>
constexpr std::ptrdiff_t dwEntityList = 0x1A1F640;
constexpr std::ptrdiff_t dwViewMatrix = 0x1A89050;
constexpr std::ptrdiff_t dwLocalPlayerController = 0x1A6E8D0;
constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x1874050;
constexpr std::ptrdiff_t m_iTeamNum = 0x3E3; // uint8
constexpr std::ptrdiff_t m_fFlags = 0x3EC; // uint32
constexpr std::ptrdiff_t m_iHealth = 0x344; // int32
constexpr std::ptrdiff_t m_hPlayerPawn = 0x814; // CHandle<C_CSPlayerPawn>
constexpr std::ptrdiff_t m_vOldOrigin = 0x1324; // Vector
constexpr std::ptrdiff_t m_vecViewOffset = 0xCB0; // CNetworkViewOffsetVector


