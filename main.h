#pragma once

#include "tslib/tslib.h"

#include "engine.h"
#include <vector>
#include <filesystem>

constexpr tslog::level LOG_LEVEL_TARGET = tslog::level::LOG; // { 0:VERBOSE, 1:LOG, 2:INFO, 3:WARNINGS, 4:ERRORS, 5:CRITICAL, 6:DEBUG, 7:OFF }
constexpr bool SHOULD_ALLOC_CONSOLE = true;

#define safe_release(x) if(x) { x->Release(); x = nullptr; }
namespace fs = std::filesystem;

static DWORD mSize = 0;
static uintptr_t dxgi = NULL;
static HINSTANCE g_hInstance = NULL;
static bool g_Running = false;
static bool g_ShowMenu = false;
static HWND g_TargetWindow = NULL;
static MODULEINFO g_BaseModule;

static ID3D11Device* device = nullptr;
static ID3D11DeviceContext* context = nullptr;
static ID3D11RenderTargetView* rtv = nullptr;

static int tPirateSeedN = -1;
static char tPirateSeed[0x64] = { 0 };
static int tPirateGender = -1;
static int tPirateEthnicity = -1;
static float tPirateAge = -1.f;
static float tPirateBodyType = -1.f;
static float tPirateBodyTypeModifier = -1.f;
static float tPirateDirtiness = -1.f;
static float tPirateWonkiness = -1.f;


bool init();
bool end();
void run();
bool getBaseModule();
void getModule();
void hookPresent();
void hookResizeBuffer();
void clearDX11Objects();
uintptr_t findGWorld();
uintptr_t findGObjects();
uintptr_t findGNames();


HRESULT presentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
typedef HRESULT(*DX11Present)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
DX11Present oDX11Present = nullptr;

HRESULT resizeBufferHook(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);
typedef HRESULT(*DX11ResizeBuffer)(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);
DX11ResizeBuffer oDX11ResizeBuffer = nullptr;

// LRESULT CALLBACK windowProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC oWindowProc = nullptr;

LRESULT CALLBACK windowProcHookEx(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// WINDOWS DEBUG HELPERS

HCURSOR WINAPI hkSetCursor(HCURSOR hCursor);
BOOL WINAPI hkSetCursorPos(int x, int y);
typedef HCURSOR(WINAPI* fnSetCursor)(HCURSOR hCursor);
typedef BOOL(WINAPI* fnSetCursorPos)(int x, int y);
fnSetCursor oSetCursor = nullptr;
fnSetCursorPos oSetCursorPos = nullptr;

/*
BOOL WINAPI hkClipCursor(const RECT* lpRect);
typedef BOOL(WINAPI* fnClipCursor)(const RECT* lpRect);
fnClipCursor oClipCursor;

BOOL WINAPI hkReleaseCursor();
typedef BOOL(WINAPI* fnReleaseCursor)();
fnReleaseCursor oReleaseCursor;

HWND WINAPI hkSetCapture(HWND hWnd);
typedef HWND(WINAPI* fnSetCapture)(HWND hWnd);
fnSetCapture oSetCapture;

BOOL WINAPI hkGetClipCursor(LPRECT lpRect);
typedef BOOL(WINAPI* fnGetClipCursor)(LPRECT lpRect);
fnGetClipCursor oGetClipCursor;
*/
// END WINDOWS DEBUG HELPERS 
