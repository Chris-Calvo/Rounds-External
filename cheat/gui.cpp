#include "gui.h"
#include "ProcessMemory.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND	window,
	UINT	message,
	WPARAM	wideParameter,
	LPARAM	longParamter
);

LRESULT __stdcall WindowProcess(HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter) {

	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter)) {
		return true;
	}

	switch (message) {

	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED) {
			gui::presentParams.BackBufferWidth = LOWORD(longParameter);
			gui::presentParams.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	} return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) {
			return 0;
		}
	} break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	} return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter);
	} return 0;
	
	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON) {
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{};

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 &&
				gui::position.y <= 19) {

				SetWindowPos(gui::window, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
			}
		}
	} return 0;
	}
	return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName, const char* className) noexcept {
	
	windowClass.cbSize = sizeof(WNDCLASSEXA);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = className;
	windowClass.hIconSm = 0;

	RegisterClassExA(&windowClass);

	window = CreateWindowA(className, windowName, WS_POPUP, 100, 100, WIDTH, HEIGHT, 0, 0, windowClass.hInstance, 0);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}


void gui::DestroyHWindow() noexcept {

	DestroyWindow(window);
	UnregisterClassA(windowClass.lpszClassName, windowClass.hInstance);
}


bool gui::CreateDevice() noexcept {

	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d) {
		return false;
	}

	ZeroMemory(&presentParams, sizeof(presentParams));

	presentParams.Windowed = TRUE;
	presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParams.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParams.EnableAutoDepthStencil = TRUE;
	presentParams.AutoDepthStencilFormat = D3DFMT_D16;
	presentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParams, &device) < 0) {
		return false;
	}

	return true;
}

void gui::ResetDevice() noexcept {

	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParams);

	if (result == D3DERR_INVALIDCALL) {
		IM_ASSERT(0);
	}

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept {

	if (device) {
		device->Release();
		device = nullptr;
	}

	if (d3d) {
		d3d->Release();
		d3d = nullptr;
	}
}



void gui::CreateImGui() noexcept {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept {

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


void gui::BeginRender() noexcept {
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept {

	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
		ResetDevice();
	}
}

static float sliderValue = 100.0f;
float currentHealth = 0.0f;
bool jumpCheck = false;
int jumpOn = 2;
int jumpOff = 1;
float move = 1;
int respawn = 0;
bool blockCheck = false;
float blockOn = 0;
float blockOff = 4;
float reloadSpeed = 2;
float damage = 1.0f;
float kback = 1.0f;
float fireRate = 0.3f;
float bulletSpeed = 1.0;
int bulletBounceCount = 0;
float bulletSize = 1.0f;
int ammoCount = 3;

bool IgnoreWallCheck = false;
byte ignoreWallsOn = 1;
byte ignoreWallsOff = 0;


std::vector<uintptr_t> healthOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x11C };
std::vector<uintptr_t> jumpOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x108 };
std::vector<uintptr_t> moveOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x80, 0x88 };
std::vector<uintptr_t> respawnOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x80, 0xB8 };
std::vector<uintptr_t> blockOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x78, 0xB4 };

std::vector<uintptr_t> reloadOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x88, 0x28, 0x78, 74 };
std::vector<uintptr_t> damageOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x88, 0x28, 0xB0 };
std::vector<uintptr_t> kbackOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x88, 0x28, 0xB0 };
std::vector<uintptr_t> fireRateOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x88, 0x28, 0xC0 };
std::vector<uintptr_t> bulletSpeedOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x88, 0x28, 0xC4 };
std::vector<uintptr_t> bulletBounceOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x88, 0x28, 0xFC };
std::vector<uintptr_t> bulletSizeOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x88, 0x28, 0xE8};
std::vector<uintptr_t> ignoreWallOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x88, 0x28, 0x15A };

std::vector<uintptr_t> ammoCountOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x88, 0x28, 0x78, 0x68 };
//std::vector<uintptr_t> reloadSpeedOffsets = { 0x1CB3A80, 0x8, 0x50, 0x48, 0x40, 0xF8, 0xA0, 0x88, 0x28, 0x78, 0x74 };

void gui::Render() noexcept {

	ProcessMemory processMemory;
	processMemory.GetProcessHandle(L"ROUNDS.exe");

	uintptr_t baseAddress = processMemory.GetModuleBaseAddress(L"UnityPlayer.dll");

	uintptr_t healthAddress = processMemory.ResolvePointer(baseAddress, healthOffsets);
	uintptr_t jumpAddress = processMemory.ResolvePointer(baseAddress, jumpOffsets);
	uintptr_t moveAddress = processMemory.ResolvePointer(baseAddress, moveOffsets);
	uintptr_t respawnAddress = processMemory.ResolvePointer(baseAddress, respawnOffsets);
	uintptr_t blockAddress = processMemory.ResolvePointer(baseAddress, blockOffsets);
	uintptr_t reloadSpeedAddress = processMemory.ResolvePointer(baseAddress, blockOffsets);
	uintptr_t damageAddress = processMemory.ResolvePointer(baseAddress, damageOffsets);
	uintptr_t kbackAddress = processMemory.ResolvePointer(baseAddress, kbackOffsets);
	uintptr_t fireRateAddress = processMemory.ResolvePointer(baseAddress, fireRateOffsets);
	uintptr_t bulletSpeedAddress = processMemory.ResolvePointer(baseAddress, bulletSpeedOffsets);
	uintptr_t bulletBounceAddress = processMemory.ResolvePointer(baseAddress, bulletBounceOffsets);
	uintptr_t bulletSizeAddress = processMemory.ResolvePointer(baseAddress, bulletSizeOffsets);
	uintptr_t ammoCountAddress = processMemory.ResolvePointer(baseAddress, ammoCountOffsets);
	uintptr_t ignoreWallAddress = processMemory.ResolvePointer(baseAddress, ignoreWallOffsets);


	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({WIDTH, HEIGHT});
	ImGui::Begin(
		"Skuz's ROUNDS Menu",
		&exit,
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoSavedSettings | 
		ImGuiWindowFlags_NoCollapse | 
		ImGuiWindowFlags_NoMove
	);

	processMemory.ReadMemory(healthAddress, currentHealth);
	
	if (ImGui::BeginTabBar("MyTabBar"))
	{
		if (ImGui::BeginTabItem("Player Attibutes")) {

			ImGui::Text("Current Health: %.2f", currentHealth);
			if (ImGui::Button("Set Health")) {
				processMemory.WriteMemory(healthAddress, sliderValue);
			}
			ImGui::SameLine(160);
			ImGui::SliderFloat("##HealthSlider", &sliderValue, 0.0f, 1000.0f);

			if (ImGui::Button("Set Movement Speed")) {
				processMemory.WriteMemory(moveAddress, move);
			}
			ImGui::SameLine(160);
			ImGui::InputFloat("##MS", &move, 0.5f);

			if (ImGui::Button("Set Respawns")) {
				processMemory.WriteMemory(respawnAddress, respawn);
			}
			ImGui::SameLine(160);
			ImGui::InputInt("##R", &respawn);

			if (ImGui::Checkbox("Infinite Jump", &jumpCheck)) {
				if (jumpCheck) {
					processMemory.WriteMemory(jumpAddress, jumpOn);
				}
				else {
					processMemory.WriteMemory(jumpAddress, jumpOff);
				}
			}

			ImGui::SameLine(160);

			if (ImGui::Checkbox("Infinite Block", &blockCheck)) {
				if (blockCheck) {
					processMemory.WriteMemory(blockAddress, blockOn);
				}
				else {
					processMemory.WriteMemory(blockAddress, blockOff);
				}
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Gun Attributes"))
		{
			if (ImGui::Button("Set Reload Speed")) {
				processMemory.WriteMemory(reloadSpeedAddress, reloadSpeed);
			}
			ImGui::SameLine(160);
			ImGui::InputFloat("##RS", &reloadSpeed, 1.0f);

			if (ImGui::Button("Set Damage")) {
				processMemory.WriteMemory(reloadSpeedAddress, damage);
			}
			ImGui::SameLine(160);
			ImGui::InputFloat("##DAM", &damage, 1.0f);

			if (ImGui::Button("Set Knockback")) {
				processMemory.WriteMemory(kbackAddress, kback);
			}
			ImGui::SameLine(160);
			ImGui::InputFloat("##KNOCK", &kback, 1.0f);

			if (ImGui::Button("Set Fire Rate")) {
				processMemory.WriteMemory(fireRateAddress, fireRate);
			}
			ImGui::SameLine(160);
			ImGui::InputFloat("##FIRE", &fireRate, 0.3f);

			if (ImGui::Button("Set Bullet Speed")) {
				processMemory.WriteMemory(bulletSpeedAddress, bulletSpeed);
			}
			ImGui::SameLine(160);
			ImGui::InputFloat("##BSpeed", &bulletSpeed, 0.5f);

			if (ImGui::Button("Set Bullet Bounces")) {
				processMemory.WriteMemory(bulletBounceAddress, bulletBounceCount);
			}
			ImGui::SameLine(160);
			ImGui::InputInt("##BBounce", &bulletBounceCount);

			if (ImGui::Button("Set Bullet Size")) {
				processMemory.WriteMemory(bulletSizeAddress, bulletSize);
			}
			ImGui::SameLine(160);
			ImGui::InputFloat("##BSize", &bulletSize, 0.5f);

			if (ImGui::Button("Set Max Ammo")) {
				processMemory.WriteMemory(ammoCountAddress, ammoCount);
			}
			ImGui::SameLine(160);
			ImGui::InputInt("##ACount", &ammoCount, 2);

			if (ImGui::Checkbox("Ignore Walls", &IgnoreWallCheck)) {
				if (IgnoreWallCheck) {
					processMemory.WriteMemory(ignoreWallAddress, ignoreWallsOn);
				}
				else {
					processMemory.WriteMemory(ignoreWallAddress, ignoreWallsOff);
				}
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}


	ImGui::End();
}
