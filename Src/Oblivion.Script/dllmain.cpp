#include "Stdafx.h"

#pragma unmanaged

void GetOblivionSearchString(std::string & search_string){
	char output[256]; 
	GetPrivateProfileString("Loader","RuntimeName","Oblivion.exe",&output[0],256,".\\Data\\OBSE\\obse.ini");
	search_string = std::string(&output[0]);
}

void OblivionPluginInit(HMODULE hModule)
{
	HMODULE hOblivion = GetModuleHandleA("Oblivion.Online.dll");
	CallOblivionFunction = (TCallOblivionFunction)GetProcAddress(hOblivion, "CallFunction");

	if(!CallOblivionFunction)
	{
		MessageBoxA(0,"Can't load the function !","Error",0);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			std::string strL;
			std::string searchString;
			strL.resize(MAX_PATH);
			GetModuleFileNameA(NULL, &strL[0], MAX_PATH) ;

			DisableThreadLibraryCalls((HMODULE)hModule);
			
			GetOblivionSearchString(searchString);
			if(strL.find(searchString) != std::string::npos)
			{
				OblivionPluginInit(hModule);
			}
			break;
		}
	case DLL_PROCESS_DETACH:
		{		
			break;
		}
	}
	return TRUE;
}