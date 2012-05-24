// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

HMODULE g_hModule = NULL;

void WritePipe(LPCTSTR psz);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	    case DLL_PROCESS_ATTACH:
        {
            g_hModule = hModule;

            //CString szFileName;
            //::GetModuleFileName(NULL, CStrBuf(szFileName, 65535), 65535);
            //WritePipe((LPCTSTR)szFileName);
        }
	    case DLL_THREAD_ATTACH:
	    case DLL_THREAD_DETACH:
	    case DLL_PROCESS_DETACH:
		    break;
	}
	return TRUE;
}
