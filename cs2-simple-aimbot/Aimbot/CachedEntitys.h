#pragma once
#include "Globals.h"
#include "Offsets.h"
#include <vector>
#include <chrono>
#include <ctime>
using namespace std::chrono;

bool ShootAllPlayers = true;

struct Entity
{
	uintptr_t address;
	int Team;
};

std::vector<Entity> cachedEntitys;
uintptr_t localPlayerCached = NULL;

void UpdateCachedEntitys()
{
	static auto start_time = steady_clock::now();

	auto current_time = steady_clock::now();

	auto elapsed_time = duration_cast<seconds>(current_time - start_time);


	if (elapsed_time < seconds(5))
	{
		return;
	}

	start_time = current_time;
	cachedEntitys.clear();


	for (int i = 1; i < 64; i++)
	{
		uintptr_t entity_list = ReadMemory< uintptr_t >(Globals::clientDLL + dwEntityList);


		uintptr_t listEntry = ReadMemory< uintptr_t >(entity_list + ((8 * (i & 0x7FFF) >> 9) + 16));
		if (!listEntry) continue;

		uintptr_t entityController = ReadMemory< uintptr_t >(listEntry + 120 * (i & 0x1FF));
		if (!entityController) continue;

		uintptr_t entityControllerPawn = ReadMemory< uintptr_t >(entityController + m_hPlayerPawn);
		if (!entityControllerPawn) continue;

		uintptr_t list_entry2 = ReadMemory<uintptr_t>(entity_list + 0x8 * ((entityControllerPawn & 0x7FFF) >> 9) + 16);


		uintptr_t entity = ReadMemory< uintptr_t >(list_entry2 + 120 * (entityControllerPawn & 0x1FF));
		if (!entity) continue;

		uintptr_t localPlayerCached = ReadMemory< uintptr_t >(Globals::clientDLL + dwLocalPlayerPawn);
		int localPlayerTeamCached2 = ReadMemory< int >(localPlayerCached + m_iTeamNum);

		if (entity == localPlayerCached) {
			continue;
		}

		int entityTeam = ReadMemory< int >(entity + m_iTeamNum);


		if (!ShootAllPlayers)
		{
			if (localPlayerTeamCached2 == entityTeam)
				continue;
		}


		Entity ent = { entity, entityTeam };
		cachedEntitys.push_back(ent);

	}
}