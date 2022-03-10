// Dear ImGui: standalone example application for DirectX 9
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <math.h>
#include <bitset>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

using namespace std;

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
string convert_type[2] = { "Decimal", "Binary" };
string input_type[8] = { "Enter decimal number:", "Enter binary signed number:", "Binary signed number:", "Or Enter binary signed 1's complement:"
,"Binary signed 1's complement:" ,"Or Enter binary signed 2's complement:"
, "Binary signed 2's complement:" ,"Decimal number" };
bool swapval = false;
string input_tmp;
char input[1000000];
char input_bi_1_comp[1000000];
char input_bi_2_comp[1000000];
string output;
string output_1_comp;
string output_2_comp;
string output_dec;
int selected_fish = 0;
int bitlength = 8;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Simple helper function to load an image into a DX9 texture with common settings
bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height)
{
	// Load texture from disk
	PDIRECT3DTEXTURE9 texture;
	HRESULT hr = D3DXCreateTextureFromFileA(g_pd3dDevice, filename, &texture);
	if (hr != S_OK)
		return false;

	// Retrieve description of the texture surface so we can access its size
	D3DSURFACE_DESC my_image_desc;
	texture->GetLevelDesc(0, &my_image_desc);
	*out_texture = texture;
	*out_width = (int)my_image_desc.Width;
	*out_height = (int)my_image_desc.Height;
	return true;
}

void GetDesktopResolution(int& horizontal, int& vertical)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
}
int my_image_width = 0;
int my_image_height = 0;
PDIRECT3DTEXTURE9 my_texture = NULL;

void changelength(int size) {
	if (size <= 8) {
		bitlength = 8;
	}
	else if (size > 8 && size <= 12) {
		bitlength = 12;
	}
	else if (size > 12 && size <= 16) {
		bitlength = 16;
	}
	else if (size > 16 && size <= 20) {
		bitlength = 20;
	}
	else if (size > 20 && size <= 24) {
		bitlength = 24;
	}
	else if (size > 24 && size <= 28) {
		bitlength = 28;
	}
	else if (size > 28 && size <= 32) {
		bitlength = 32;
	}
	else if (size > 32 && size <= 36) {
		bitlength = 36;
	}
	else if (size > 36 && size <= 40) {
		bitlength = 40;
	}
	else if (size > 40 && size <= 44) {
		bitlength = 44;
	}
	else if (size > 44 && size <= 48) {
		bitlength = 48;
	}
	else if (size > 48 && size <= 52) {
		bitlength = 52;
	}
	else if (size > 52 && size <= 56) {
		bitlength = 56;
	}
	else if (size > 56 && size <= 60) {
		bitlength = 60;
	}
}
string addBinary(string A, string B)
{
	// If the length of string A is greater than the length
	// of B then just swap the the string by calling the
	// same function and make sure to return the function
	// otherwise recursion will occur which leads to
	// calling the same function twice
	if (A.length() > B.length())
		return addBinary(B, A);

	// Calculating the differnce between the length of the
	// two strings.
	int diff = B.length() - A.length();

	// Initialise the padding string which is used to store
	// zeroes that should be added as prefix to the string
	// which has length smaller than the other string.
	string padding;
	for (int i = 0; i < diff; i++)
		padding.push_back('0');

	A = padding + A;
	string res;
	char carry = '0';

	for (int i = A.length() - 1; i >= 0; i--) {
		// This if condition solves 110 111 possible cases
		if (A[i] == '1' && B[i] == '1') {
			if (carry == '1')
				res.push_back('1'), carry = '1';
			else
				res.push_back('0'), carry = '1';
		}
		// This if condition solves 000 001 possible cases
		else if (A[i] == '0' && B[i] == '0') {
			if (carry == '1')
				res.push_back('1'), carry = '0';
			else
				res.push_back('0'), carry = '0';
		}
		// This if condition solves 100 101 010 011 possible
		// cases
		else if (A[i] != B[i]) {
			if (carry == '1')
				res.push_back('0'), carry = '1';
			else
				res.push_back('1'), carry = '0';
		}
	}

	// If at the end their is carry then just add it to the
	// result
	if (carry == '1')
		res.push_back(carry);
	// reverse the result
	reverse(res.begin(), res.end());

	// To remove leading zeroes
	int index = 0;
	while (index + 1 < res.length() && res[index] == '0')
		index++;
	return (res.substr(index));
}
void DecimalToBinary(string input) {
	output.clear();
	output_1_comp.clear();
	output_2_comp.clear();
	if (input == "0" || input == "+0") {
		bitlength = 8;
		for (int i = 0; i < bitlength; i++) {
			output.push_back('0');
			output_1_comp.push_back('0');
			output_2_comp.push_back('0');
		}

	}
	else if (input == "-0") {
		bitlength = 8;
		output.push_back('1');
		for (int i = 0; i < bitlength - 1; i++) {
			output.push_back('0');
		}
		for (int i = 0; i < bitlength; i++) {
			output_1_comp.push_back('1');
			output_2_comp.push_back('0');
		}
	}
	else {
		double tmp = stod(input);
		if (input[0] == '-') tmp = -tmp;
		long long nguyen = (long long)tmp;
		double du = tmp - nguyen;

		while (nguyen > 0) {
			char tmp = nguyen % 2 + '0';
			output.push_back(tmp);
			nguyen /= 2;
		}
		reverse(output.begin(), output.end());
		if (du > 0) {
			output.push_back('.');
			while (du * 2 != 0) {
				du *= 2;
				if (du >= 1) output.push_back('1');
				else output.push_back('0');
				du = du - (int)du;
			}
			output_1_comp = "N/A";
			output_2_comp = "N/A";
		}
		changelength(output.size());
		reverse(output.begin(), output.end());
		int len_tmp = bitlength - output.size();
		for (int i = 0; i < len_tmp; i++)
			output.push_back('0');

		if (input[0] == '-') {
			output.pop_back();
			output.push_back('1');
		}
		reverse(output.begin(), output.end());
		if (input.find(".") == string::npos) {
			output_1_comp.clear();
			output_2_comp.clear();
			if (input[0] == '-') {
				output_1_comp.push_back('1');
				for (int i = 1; i < output.size(); i++) {
					if (output[i] == '0') output_1_comp.push_back('1');
					if (output[i] == '1') output_1_comp.push_back('0');
				}
				output_2_comp = addBinary(output_1_comp, "1");
			}
			else {
				output_1_comp = output;
				output_2_comp = output;

			}
		}
		else {
			if (input[0] == '-') output = "N/A";
		}
	}
}
string bi_1_comp_to_bi(string input) {
	string res;
	res.push_back('1');
	for (int i = 1; i < input.size(); i++) {
		if (input[i] == '0') res.push_back('1');
		if (input[i] == '1') res.push_back('0');
	}
	return res;
}
void BinaryToDecimal(string input, int mode) {
	output_dec.clear();
	string input_tmp, input_nguyen, input_du;
	if (mode == 0) input_tmp = input;
	else if (mode == 1) {
		if (input.find('.') != string::npos) return;
		if (input[0] == '1')
			input_tmp = bi_1_comp_to_bi(input);
		else
			input_tmp = input;
	}
	size_t found = input_tmp.find(".");
	if (found != string::npos) {
		input_nguyen = input_tmp.substr(0, found);
		input_du = input_tmp.substr(found + 1, input_tmp.size() - 1);
	}
	reverse(input_tmp.begin(), input_tmp.end());
	reverse(input_nguyen.begin(), input_nguyen.end());
	double res = 0;
	if (found == string::npos) {
		for (int i = 0; i < input_tmp.size() - 1; i++) {
			if (input_tmp[i] == '1')
				res += pow(2, i);
		}
	}
	else {
		for (int i = 0; i < input_nguyen.size() - 1; i++) {
			if (input_nguyen[i] == '1')
				res += pow(2, i);
		}
		for (int i = 0; i < input_du.size(); i++) {
			if (input_du[i] == '1')
				res += pow(2, -(i + 1));
		}
	}

	std::stringstream stream;
	int count = 0;
	double du = res - (long long)res;
	while (du != 0) {
		du *= 10;
		count++;
		du = du - (long long)du;
	}
	if (input_tmp[input_tmp.size() - 1] == '1') res = -res;
	stream << std::fixed << std::setprecision(count) << res;
	output_dec = stream.str();
}
void Reset() {
	input[0] = '\0';
	input_bi_1_comp[0] = '\0';
	input_bi_2_comp[0] = '\0';
	output.clear();
	output_1_comp.clear();
	output_2_comp.clear();
	output_dec.clear();
}
// Main code
int main(int, char**)
{
	int hwnd_pos_x, hwnd_pos_y;
	GetDesktopResolution(hwnd_pos_x, hwnd_pos_y);
	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Number Conversion"), NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Number Conversion"), WS_OVERLAPPED |
		WS_CAPTION |
		WS_SYSMENU |
		WS_THICKFRAME |
		WS_MINIMIZEBOX, (hwnd_pos_x - 632) / 2, (hwnd_pos_y - 660) / 2, 632, 660, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic(); 

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 7.0f;

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Tahoma.ttf", 22.0f, NULL, io.Fonts->GetGlyphRangesDefault());
	io.Fonts->AddFontFromFileTTF("Pacifico-Regular.ttf", 40.0f, NULL, io.Fonts->GetGlyphRangesDefault());
	io.Fonts->AddFontFromFileTTF("Pacifico-Regular.ttf", 100.0f, NULL, io.Fonts->GetGlyphRangesDefault());
	io.Fonts->AddFontFromFileTTF("Caramel and Vanilla.ttf", 22.0f, NULL, io.Fonts->GetGlyphRangesDefault());

	bool ret = LoadTextureFromFile("swap.png", &my_texture, &my_image_width, &my_image_height);
	IM_ASSERT(ret);
	//IM_ASSERT(font != NULL);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	bool done = false;
	while (!done)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// See the WndProc() function below for our to dispatch events to the Win32 backend.
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		// Start the Dear ImGui frame
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::Begin("Number Conversion", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::PushFont(io.Fonts->Fonts[2]);
			ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_FrameBg);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGuiCol_FrameBg);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGuiCol_FrameBg);
			ImGui::Button("Number Conversion", ImVec2(600, 0));
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopFont();
			ImGui::Separator();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::Button(convert_type[swapval].c_str(), ImVec2(240, 55));
			ImGui::SameLine();
			ImGui::SetCursorPosX(278);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 25.0f);
			if (ImGui::Button("###Swap", ImVec2(62, 55))) { swapval = !swapval; Reset(); }
			ImGui::PopStyleVar();
			ImGui::SameLine();
			ImGui::SetCursorPosX(368);
			ImGui::Button(convert_type[!swapval].c_str(), ImVec2(240, 55));
			ImGui::SetCursorPos(ImVec2(277, 117));
			ImGui::Image((void*)my_texture, ImVec2(my_image_width, my_image_height));
			ImGui::PopFont();
			ImGui::Text(input_type[swapval].c_str());
			ImGui::PushItemWidth(600);
			ImGui::InputText("##input", input, IM_ARRAYSIZE(input), ImGuiInputTextFlags_CharsDecimal);
			if (ImGui::IsItemActive() && swapval) {
				if (input[0] != '\0') {
					input_bi_1_comp[0] = '\0';
					input_bi_2_comp[0] = '\0';
				}
			}
			if (!swapval) {
				if (ImGui::Button("Convert", ImVec2(100, 40))) DecimalToBinary(input);
				ImGui::SameLine();
				if (ImGui::Button("Reset", ImVec2(100, 40))) Reset();
			}
			ImGui::Text(input_type[swapval + 2].c_str());
			ImGui::InputTextMultiline("##input_output", !swapval ? (char*)output.c_str() : input_bi_1_comp, !swapval ? output.size() + 1 : IM_ARRAYSIZE(input_bi_1_comp), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3), !swapval ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_CharsDecimal);
			if (ImGui::IsItemActive() && swapval) {
				if (input_bi_1_comp[0] != '\0') {
					input[0] = '\0';
					input_bi_2_comp[0] = '\0';
				}
			}

			if (!swapval) {
				ImGui::Text(input_type[swapval + 4].c_str());
				ImGui::InputTextMultiline("##1scomp", !swapval ? (char*)output_1_comp.c_str() : input_bi_2_comp, !swapval ? output_1_comp.size() + 1 : IM_ARRAYSIZE(input_bi_2_comp), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3), !swapval ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_CharsDecimal);
				if (ImGui::IsItemActive() && swapval) {
					if (input_bi_2_comp[0] != '\0') {
						input[0] = '\0';
						input_bi_1_comp[0] = '\0';
					}
				}
			}
			if (swapval) {
				if (ImGui::Button("Convert", ImVec2(100, 40))) {
					if (input[0] != '\0')
						BinaryToDecimal(input, 0);
					else if (input_bi_1_comp[0] != '\0')
						BinaryToDecimal(input_bi_1_comp, 1);
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset", ImVec2(100, 40))) Reset();
			}
			ImGui::Text(input_type[swapval + 6].c_str());
			ImGui::InputTextMultiline("##2scomp", !swapval ? (char*)output_2_comp.c_str() : (char*)output_dec.c_str(), !swapval ? output_2_comp.size() + 1 : output_dec.size() + 1, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3), ImGuiInputTextFlags_ReadOnly);
			ImGui::Separator();
			if (!swapval)
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Bit Length: %d bit", bitlength);

			ImGui::End();
		}

		// Rendering
		ImGui::EndFrame();
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}
		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		// Handle loss of D3D9 device
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
