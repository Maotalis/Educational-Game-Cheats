#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "Vector.h"
#include "memory.h"
#include "offsets.h"
#include <vector>
#include <chrono>
#include <thread>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
using namespace std::chrono;

struct Entity
{
	uintptr_t address;
	int Team;
};


HWND hwnd = nullptr;
ID3D11Device* pDevice = nullptr;
ID3D11DeviceContext* pContext = nullptr;
IDXGISwapChain* pSwapChain = nullptr;
ID3D11RenderTargetView* pRenderTargetView = nullptr;
bool isProgramAlive = true;
std::vector<Entity> cachedEntitys;
DWORD localPlayerCached = NULL;
int localPlayerTeamCached = NULL;
auto lastTime = steady_clock::now();
float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Render();
void ESP();
void UpdateCachedEntitys();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!Initialize())
		return false;

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance, NULL, NULL, NULL, NULL, L"ImGui Overlay", NULL };
	RegisterClassEx(&wc);

	hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, wc.lpszClassName, L"ImGui Overlay", WS_POPUP, 0, 0, screenWidth, screenHeight, NULL, NULL, hInstance, NULL);
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY | LWA_ALPHA);
	ShowWindow(hwnd, SW_SHOW);

	DXGI_SWAP_CHAIN_DESC scDesc = {};
	scDesc.BufferCount = 2;                           // Number of buffers in the swap chain
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Buffer usage as a render target
	scDesc.OutputWindow = hwnd;                       // Link swap chain to the application window
	scDesc.SampleDesc.Count = 1;                      // Multisampling disabled (set to 1 for no MSAA)
	scDesc.SampleDesc.Quality = 0;                    // Default quality level
	scDesc.Windowed = TRUE;                           // Enable windowed mode
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;     // Discard back buffer after presenting
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32-bit color format

	// Handle common DirectX initialization issues
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
		&scDesc, &pSwapChain, &pDevice, NULL, &pContext );

	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to create device and swap chain!", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	// Retrieve the back buffer from the swap chain
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to retrieve back buffer from swap chain!", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	// Create a render target view for the back buffer
	hr = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
	pBackBuffer->Release(); // Release the back buffer after the render target view is created
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to create render target view!", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(pDevice, pContext);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	UpdateCachedEntitys();
	while (isProgramAlive)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		auto currentTime = steady_clock::now();
		auto elapsedTime = duration_cast<seconds>(currentTime - lastTime).count();
		if(elapsedTime >= 5) {
			UpdateCachedEntitys();
			lastTime = currentTime;
		}

		Render();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	}
}

void Render()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//ImGui::GetBackgroundDrawList()->AddText(ImVec2(100, 200), IM_COL32(255, 0, 0, 255), "ASIJDAISDIAJSDIASDI");
	ESP();

	ImGui::Render();
	
	pContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);
	pContext->ClearRenderTargetView(pRenderTargetView, clearColor);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	pSwapChain->Present(0, 0);
}


void ESP()
{
	auto* drawList = ImGui::GetBackgroundDrawList();
	view_matrix_t view_matrix = ReadMemory< view_matrix_t >(clientDLL + dwViewMatrix);

	for (Entity entity : cachedEntitys)
	{
		float health = ReadMemory<int>(entity.address + m_iHealth);
		if (health < 1) {
			continue;
		}

		Vector absOrigin = ReadMemory< Vector >(entity.address + m_vOldOrigin);
		Vector eyePos = { absOrigin.x , absOrigin.y , absOrigin.z + 75.f };

		Vector head, feet;
		if (WorldToScreen(view_matrix, absOrigin, head))
		{
			if (WorldToScreen(view_matrix, eyePos, feet))
			{
				float height = (head.y - feet.y);
				float width = height * 0.35f;

				drawList->AddRect({ head.x - width, head.y }, { head.x + width, feet.y }, whiteColor);


				float maxHealth = 100.0f;  // Maximum health
				float healthPercentage = (health / maxHealth) * 100.0f;

				// Health bar width and height calculations
				float barWidth = 3.0f;  // Health bar width
				float fullBarHeight = feet.y - head.y;  // Full height of the health bar (same as box height)

				ImColor healthBarColor;
				if (healthPercentage > 60.0f) {
					healthBarColor = greenColor;  // Green for health above 50%
				}
				else if (healthPercentage > 35.0f) {
					healthBarColor = yellowColor;  // Yellow for health below 50% but above 20%
				}
				else {
					healthBarColor = redColor; // Red for health below 20%
				}

				// Calculate the height of the health bar based on the health percentage
				float healthBarHeight = fullBarHeight * (healthPercentage / 100.0f);  // Proportional health height

				// Position of the health bar (left of the box) starting from the top of the box
				ImVec2 healthBarPos = ImVec2(head.x - width - 5.0f, head.y);  // 5px space between box and health bar, starting from the top

				// Draw the health bar (filled part for health) from top to bottom
				drawList->AddRectFilled(healthBarPos, ImVec2(healthBarPos.x + barWidth, healthBarPos.y + healthBarHeight), healthBarColor);  // Green bar

				// Optional: Draw the outline of the health bar
				drawList->AddRect(healthBarPos, ImVec2(healthBarPos.x + barWidth, healthBarPos.y + fullBarHeight), IM_COL32(255, 255, 255, 255));  // White outline

			}
		}
	}

}

void UpdateCachedEntitys()
{
	cachedEntitys.clear();
	uintptr_t entity_list = ReadMemory< uintptr_t >(clientDLL + dwEntityList);

	uintptr_t localPlayerCached2 = ReadMemory< uintptr_t >(clientDLL + dwLocalPlayerPawn);
	while(!localPlayerCached2)
		localPlayerCached2 = ReadMemory< uintptr_t >(clientDLL + dwLocalPlayerPawn);

	uintptr_t localPlayerTeamCached2 = ReadMemory< int >(localPlayerCached2 + m_iTeamNum);

	for (int i = 1; i < 32; i++)
	{

		uintptr_t listEntry = ReadMemory< uintptr_t >(entity_list + ((8 * (i & 0x7FFF) >> 9) + 16));
		if (!listEntry) continue;

		uintptr_t entityController = ReadMemory< uintptr_t >(listEntry + 120 * (i & 0x1FF));
		if (!entityController) continue;

		uintptr_t entityControllerPawn = ReadMemory< uintptr_t >(entityController + m_hPlayerPawn);
		if (!entityControllerPawn) continue;

		uintptr_t list_entry2 = ReadMemory<uintptr_t>(entity_list + 0x8 * ((entityControllerPawn & 0x7FFF) >> 9) + 16);


		uintptr_t entity = ReadMemory< uintptr_t >(list_entry2 + 120 * (entityControllerPawn & 0x1FF));
		if (!entity) continue;


		if (entity == localPlayerCached2) {
			continue;
		}


		int entityTeam = ReadMemory< int >(entity + m_iTeamNum);

		if (entityTeam == localPlayerTeamCached2)
			continue;

		Entity ent = { entity, entityTeam };
		cachedEntitys.push_back(ent);

	}
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_DESTROY:
		isProgramAlive = false;
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	}
}