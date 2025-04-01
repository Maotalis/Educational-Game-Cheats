#pragma once
#include "ImGui.h"

namespace Globals
{
	inline int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	inline int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	inline uintptr_t clientDLL = NULL;
	inline uintptr_t engineDLL = NULL;
	inline HANDLE hProcess = NULL;

	const ImColor whiteColor = IM_COL32(255, 255, 255, 255);
	const ImColor greenColor = IM_COL32(0, 255, 0, 255);
	const ImColor redColor = IM_COL32(255, 0, 0, 255);
	const ImColor yellowColor = IM_COL32(255, 255, 0, 255);

}