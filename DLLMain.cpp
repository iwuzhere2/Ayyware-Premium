#include "DLLMain.h"
#include "Utilities.h"

#include "INJ/ReflectiveLoader.h"

#include "Offsets.h"
#include "Interfaces.h"
#include "Hooks.h"
#include "RenderManager.h"
#include "Hacks.h"
#include "Menu.h"
#include "AntiAntiAim.h"
#include "hitmarker.h"
#include "AntiLeak.h"

#include "Dumping.h"


extern HINSTANCE hAppInstance;

float lineLBY;
float lineRealAngle;
float lineFakeAngle;
int ResolverStage[65];
bool canBacktrack;
bool BacktrackCanShotEnemy;


UCHAR szFileSys[255], szVolNameBuff[255];
DWORD dwMFL, dwSysFlags;
DWORD dwSerial;
LPCTSTR szHD = "C:\\";

HINSTANCE HThisModule;
bool DoUnload;

void CAntiLeak::ErasePE()
{
	char *pBaseAddr = (char*)GetModuleHandle(NULL);
	DWORD dwOldProtect = 0; 
	VirtualProtect(pBaseAddr, 4096, PAGE_READWRITE, &dwOldProtect);
	ZeroMemory(pBaseAddr, 4096);
	VirtualProtect(pBaseAddr, 4096, dwOldProtect, &dwOldProtect);
}

int InitialThread()
{
#ifdef Avoz_DEBUG
	Utilities::OpenConsole("Avoz");
#endif
	PrintMetaHeader();

	Offsets::Initialise();
	Interfaces::Initialise(); 
	NetVar.RetrieveClasses();
	NetvarManager::Instance()->CreateDatabase();
	Render::Initialise();
	hitmarker::singleton()->initialize();
	Hacks::SetupHacks();
	Menu::SetupMenu();
	Hooks::Initialise();
	ApplyAAAHooks();

	//Dump::DumpClassIds();


	Utilities::Log("Ready");

	while (DoUnload == false)
	{
		Sleep(1000);
	}

	Hooks::UndoHooks();
	Sleep(2000); 
	FreeLibraryAndExitThread(HThisModule, 0);

	return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		GetVolumeInformation(szHD, (LPTSTR)szVolNameBuff, 255, &dwSerial, &dwMFL, &dwSysFlags, (LPTSTR)szFileSys, 255);


		{
			{
				DisableThreadLibraryCalls(hModule);

				CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)InitialThread, NULL, NULL, NULL);

				return TRUE;
			}
		}
		return FALSE;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}
	return TRUE;
}