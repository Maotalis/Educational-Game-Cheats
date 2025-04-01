#include "Math.h"
#include "Memory.h"
#include "CachedEntitys.h"
#include <vector>
#include <iostream>



int main()
{
    std::wstring targetProcess = L"cs2.exe";
    DWORD processId = GetProcessIdByName(targetProcess);
    if (processId < 0) {
        std::cerr << "cs2.exe" << " Not Found!" << std::endl;
    }

    Globals::hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
    Globals::clientDLL = GetModuleBaseAddress(processId, L"client.dll");
    Vector CenterOfScreen = { screenWidth / 2, screenHeight / 2, 0.0f };
   

    
    while (true)
    {
        Sleep(1);

        UpdateCachedEntitys();

        if (!GetAsyncKeyState(VK_RBUTTON)) continue;


        uintptr_t localPlayerCached = ReadMemory< uintptr_t >(Globals::clientDLL + dwLocalPlayerPawn);

        Vector LocalHeadPosition = ReadMemory< Vector >(localPlayerCached + m_vOldOrigin)
            + ReadMemory< Vector >(localPlayerCached + m_vecViewOffset);


        view_matrix_t view_matrix = ReadMemory< view_matrix_t >(Globals::clientDLL + dwViewMatrix);

        float fovRadius = 50.0f;
        Vector closestEntityPosition = {};


        for (Entity entity : cachedEntitys)
        {

            int EntityHealth = ReadMemory< int >(entity.address + m_iHealth);
            if (EntityHealth < 1)
                continue;


            Vector EntityHeadPosition = ReadMemory< Vector >(entity.address + m_vOldOrigin)
                + ReadMemory< Vector >(entity.address + m_vecViewOffset);


            Vector Head;
            if (WorldToScreen(view_matrix, EntityHeadPosition, Head))
            {

                float dist = Distance(CenterOfScreen, Head);

                if (dist < fovRadius) {
                    closestEntityPosition = EntityHeadPosition;
                }
            }
        
        }

        if (!closestEntityPosition.IsZero())
        {
            Vector relativeAngle = RelativeAngles(closestEntityPosition, LocalHeadPosition);
            
            WriteMemory<Vector>(Globals::clientDLL + dwViewAngles, relativeAngle);
        }

    }

}