// proxydll.cpp

#include "stdafx.h"
#include "proxydll.h"

#include "WinAPI.hpp"
#include "Plugins.hpp"

#include "Games/Skyrim.hpp"
#include "Games/Oblivion.hpp"

HINSTANCE           gl_hOriginalDll;
HINSTANCE           gl_hThisInstance;

void LoadOriginalDll();

void Init()
{
	static bool init = true;
	if(init)
	{
		init = false;
		GetInstance()->Initialize();
	}
}

void WINAPI D3DPERF_SetOptions( DWORD dwOptions )
{
	if (!gl_hOriginalDll) LoadOriginalDll(); // looking for the "right d3d9.dll"

	// Hooking IDirect3D Object from Original Library
	typedef void (WINAPI* D3D9_Type)(DWORD dwOptions);
	D3D9_Type D3DPERF_SetOptions_fn = (D3D9_Type) GetProcAddress(
		gl_hOriginalDll, "D3DPERF_SetOptions");

	// Debug
	if (!D3DPERF_SetOptions_fn)
	{
		MessageBoxA(0,"not found :(","0",0);
	}
	return (D3DPERF_SetOptions_fn (dwOptions));
}

// Exported function (faking d3d9.dll's one-and-only export)
IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion)
{
	if (!gl_hOriginalDll) LoadOriginalDll(); // looking for the "right d3d9.dll"

	// Hooking IDirect3D Object from Original Library
	typedef IDirect3D9 *(WINAPI* D3D9_Type)(UINT SDKVersion);
	D3D9_Type D3DCreate9_fn = (D3D9_Type) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9");

    // Debug
	if (!D3DCreate9_fn)
    {
        OutputDebugString("PROXYDLL: Pointer to original D3DCreate9 function not received ERROR ****\r\n");
        ::ExitProcess(0); // exit the hard way
    }

	// Request pointer from Original Dll.
	IDirect3D9 *pIDirect3D9_orig = D3DCreate9_fn(SDKVersion);

	// Create my IDirect3D9 object and store pointer to original object there.
	// note: the object will delete itself once Ref count is zero (similar to COM objects)
	myIDirect3D9::instance = new myIDirect3D9(pIDirect3D9_orig);

	// Return pointer to hooking Object instead of "real one"
	return (myIDirect3D9::instance );
}

void GetSkyrimSearchString(std::string & search_string){
	char output[256]; 
	GetPrivateProfileString("Loader","RuntimeName","TESV.exe",&output[0],256,".\\Data\\SKSE\\skse.ini");
	search_string = std::string(&output[0]);
}

void GetOblivionSearchString(std::string & search_string){
	char output[256]; 
	GetPrivateProfileString("Loader","RuntimeName","Oblivion.exe",&output[0],256,".\\Data\\OBSE\\obse.ini");
	search_string = std::string(&output[0]);
}

std::string GetPath()
{
	char buffer[MAX_PATH];

	std::ifstream file("GameWorldProxy.def");
	if(file.is_open()) // if file
	{
		std::string name;
		file >> name;

		if(!name.empty())
		{
			std::ifstream test(name.c_str());
			if(test.is_open())
				return name;
		}
	}

	// Getting path to system dir and to d3d8.dll
	::GetSystemDirectory(buffer,MAX_PATH);

	// Append dll name
	strcat(buffer,"\\d3d9.dll");
	
	return std::string(buffer);
}

void LoadOriginalDll(void)
{
	// try to load the system's d3d9.dll, if pointer empty
	if (!gl_hOriginalDll) gl_hOriginalDll = ::LoadLibrary(GetPath().c_str());

	// Debug
	if (!gl_hOriginalDll)
	{
		OutputDebugString("PROXYDLL: Original d3d9.dll not loaded ERROR ****\r\n");
		::ExitProcess(0); // exit the hard way
	}
}

void ExitInstance()
{
    OutputDebugString("PROXYDLL: ExitInstance called.\r\n");

	// Release the system's d3d9.dll
	if (gl_hOriginalDll)
	{
		::FreeLibrary(gl_hOriginalDll);
	    gl_hOriginalDll = NULL;
	}
}

#pragma unmanaged

typedef HWND (WINAPI* tCreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
tCreateWindowExA oCreateWindowExA;

HWND WINAPI FakeCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	Init();

	return oCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

int GameType = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			std::string strL;
			std::string findStringSkyrim;
			std::string findStringOblivion;
			strL.resize(MAX_PATH);
			GetModuleFileName(NULL, &strL[0], MAX_PATH);

			DisableThreadLibraryCalls(hModule);
			
			GetSkyrimSearchString(findStringSkyrim);// use SKSE.ini to detect the correct name
			GetOblivionSearchString(findStringOblivion);// use OBSE.ini to detect the correct name
			if(strL.find(findStringSkyrim) != std::string::npos || strL.find(findStringOblivion) != std::string::npos)
			{
				gl_hThisInstance = hModule;
				
				// This block MUST run BEFORE any hook, CLR won't run otherwise.
				// Create the PluginManager
				Create();
				// Load all the plugins
				GetInstance()->Load();

				HMODULE user32 = LoadLibraryA("user32.dll");
				oCreateWindowExA = (tCreateWindowExA)DetourFunction((PBYTE)GetProcAddress(user32, "CreateWindowExA"), (PBYTE)FakeCreateWindowExA);

				if(strL.find(findStringSkyrim) != std::string::npos)
				{
					InstallSkyrim();
					GameType = 'skyr';
				}
				else //if(strL.find(findStringOblivion) != std::string::npos) // this is unnessisary as we already know it is either skyrim or oblivion
				{	
					InstallOblivion();
					GameType = 'obli';
				}

				HookDInput();
				HookWinAPI();
			}
			
			break;
		}
	case DLL_PROCESS_DETACH:
		{
			file << GetLastError();
			ReleaseWinAPI();
			ReleaseDInput();
			ExitInstance();

			break;
		}
	}
	return TRUE;
}