#include "../stdafx.h"
#include "Skyrim.hpp"
#include "../Plugins.hpp"
#include "Common.hpp"

#pragma unmanaged

typedef void (__stdcall *TWait)(int);
typedef void (__stdcall *TRegisterPlugin)(HINSTANCE);

typedef HWND(WINAPI* tCreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
tCreateWindowExA oCreateWindowExA;
typedef HANDLE(WINAPI *tCreateThread)(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, PDWORD);
tCreateThread oCreateThread;

TRegisterPlugin RegisterPlugin;		
TWait Wait;

extern HINSTANCE gl_hThisInstance;

#define SCRIPT_DRAGON "ScriptDragon.dll" 

void SkyrimPluginInit(HMODULE hModule)
{
	HMODULE hDragon = LoadLibraryA(SCRIPT_DRAGON);

	/* 
	 * In order to provide NORMAL support i need a plugins to be distributed without the DragonScript.dll engine 
	 * cuz user always must have the latest version which cud be found ONLY on my web page
	 */

	if (!hDragon) 
		return;

	RegisterPlugin = (TRegisterPlugin)GetProcAddress(hDragon, "RegisterPlugin");
	Wait = (TWait)GetProcAddress(hDragon, "WaitMs");

	if(!RegisterPlugin)
		return;

	RegisterPlugin(hModule);
}

void Init()
{
	static bool init = true;
	if (init)
	{
		init = false;
		GetInstance()->Initialize();
	}
}
HWND WINAPI FakeCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	Init();

	return oCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}
HANDLE WINAPI FakeCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, PDWORD lpThreadId)
{
	if (!GetInstance() && *(uint32_t*)lpParameter == 0x010CDD60)
	{
		// This block MUST run BEFORE any hook, CLR won't run otherwise.
		// Create the PluginManager
		Create();
		// Load all the plugins
		GetInstance()->Load();
	
		HMODULE user32 = LoadLibraryA("user32.dll");
		oCreateWindowExA = (tCreateWindowExA)DetourFunction((PBYTE)GetProcAddress(user32, "CreateWindowExA"), (PBYTE)FakeCreateWindowExA);

		SkyrimPluginInit(gl_hThisInstance);

		//::LoadLibrary("Skyrim.Script.dll");
	}

	return oCreateThread(lpThreadAttributes, dwStackSize,lpStartAddress,lpParameter,dwCreationFlags,lpThreadId);
}

void InstallSkyrim()
{
	/*
	 * Hook CreateThread and Load scriptdragon once we are sure that the unpacker is finished AKA once VMInitThread is created
	 */

	HINSTANCE kernel = GetModuleHandle("kernel32.dll");
	oCreateThread = (tCreateThread)DetourFunction((PBYTE)GetProcAddress(kernel, "CreateThread"), (PBYTE)FakeCreateThread);
}

extern "C" __declspec(dllexport) void main()
{
	SetGameScriptVariables();

	while(Wait)
	{
		GetInstance()->Run();
		Wait(0);
	}
}
