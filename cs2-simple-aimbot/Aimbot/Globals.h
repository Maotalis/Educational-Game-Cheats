#pragma once
#include <cstdint>
#include <windows.h>


namespace Globals
{
	inline uintptr_t clientDLL = NULL;
	inline uintptr_t engineDLL = NULL;
	inline HANDLE hProcess = NULL;
}