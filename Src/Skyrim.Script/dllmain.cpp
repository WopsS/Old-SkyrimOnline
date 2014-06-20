#include "Stdafx.h"
#include "common/plugin.h"

#pragma unmanaged

void GetSkyrimSearchString(std::string & search_string){
	char output[256]; 
	GetPrivateProfileString("Loader","RuntimeName","TESV.exe",&output[0],256,".\\Data\\SKSE\\skse.ini");
	search_string = std::string(&output[0]);
}

extern "C" __declspec(dllexport) void main()
{
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
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
			
			GetSkyrimSearchString(searchString);
			
			if(strL.find(searchString) != std::string::npos)
			{
				SkyrimPluginInit(hModule);
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