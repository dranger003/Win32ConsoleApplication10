// Win32ConsoleApplication3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <conio.h>
#include <Windows.h>

#ifdef _WIN64
    #pragma comment(lib, "..\\x64\\Debug\\Win32Project1.lib")
#else
    #pragma comment(lib, "..\\Debug\\Win32Project1.lib")
#endif

#include <AclAPI.h>
#pragma comment(lib, "AdvAPI32.Lib")

#define NAMED_PIPE          _T("\\\\.\\pipe\\F09DC44C-33F3-432D-BE51-FF56194F062A")
#define PIPES               2
#define BUF_SIZE            512

extern "C" __declspec(dllimport) DWORD InstallHook();
extern "C" __declspec(dllimport) DWORD UninstallHook();

//VOID CALLBACK WorkCallback2(
//    PTP_CALLBACK_INSTANCE ptpci,
//    PVOID pvCtx,
//    PTP_WORK ptpw)
//{
//}
//
//VOID CALLBACK WorkCallback1(
//    PTP_CALLBACK_INSTANCE ptpci,
//    PVOID pvCtx,
//    PTP_WORK ptpw)
//{
//    CHandle hPipe = (CHandle)::CreateNamedPipe(
//        NAMED_PIPE,
//        PIPE_ACCESS_INBOUND,
//        PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE | PIPE_REJECT_REMOTE_CLIENTS,
//        1,
//        4096,
//        4096,
//        0,
//        NULL);
//
//    ATLASSERT(hPipe != INVALID_HANDLE_VALUE);
//
//    ::_tprintf(_T("Waiting...\n"));
//
//    TCHAR szBuf[BUF_SIZE] = { 0 };
//    DWORD dwBytes = 0;
//
//    while (::_tcscmp(szBuf, _T("QUIT")) != 0)
//    {
//        if (::ConnectNamedPipe(hPipe, NULL))
//        {
//            PTP_WORK ptpw = ::CreateThreadpoolWork(&WorkCallback2, (PVOID)&hPipe, (PTP_CALLBACK_ENVIRON)pvCtx);
//            ::SubmitThreadpoolWork(ptpw);
//        }
//    }
//
//    ::_tprintf(_T("\nClosing.\n"));
//}

typedef struct _OVERLAPPEDEX
{
    OVERLAPPED Ovl;
    CHandle hPipe;
    BOOL bIoPending;
    DWORD dwState;
} OVERLAPPEDEX, *POVERLAPPEDEX;

DWORD CALLBACK ThreadProc(PVOID pv)
{
    HANDLE hEvent = *(PHANDLE)pv;

    HANDLE hEvents[PIPES];
    OVERLAPPEDEX Ovlxs[PIPES];

    SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
    PSID psid = NULL;

    ATLASSERT(::AllocateAndInitializeSid(
        &sia,
        1,
        SECURITY_WORLD_RID,
        0,
        0,
        0,
        00,
        0,
        0,
        0,
        &psid));

    EXPLICIT_ACCESS ea = { 0 };
    ea.grfAccessPermissions = FILE_ALL_ACCESS;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPTSTR)psid;

    PACL pacl = NULL;
    ATLASSERT(::SetEntriesInAcl(1, &ea, NULL, &pacl) == ERROR_SUCCESS);

    PSECURITY_DESCRIPTOR psd = ::LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    ATLASSERT(::InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION));

    ATLASSERT(::SetSecurityDescriptorDacl(psd, TRUE, pacl, FALSE));

    SECURITY_ATTRIBUTES sa = { 0 };
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = psd;
    sa.bInheritHandle = FALSE;

    for (int i = 0; i < PIPES; i++)
    {
        ZeroMemory(&Ovlxs[i], sizeof(OVERLAPPEDEX));

        Ovlxs[i].hPipe = (CHandle)::CreateNamedPipe(
            NAMED_PIPE,
            PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
            PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE,
            PIPES,
            BUF_SIZE,
            BUF_SIZE,
            2500,
            &sa);

        //::_tprintf(_T("CreateNamedPipe(): %ld\n"), ::GetLastError());
        ATLASSERT(Ovlxs[i].hPipe != INVALID_HANDLE_VALUE);

        hEvents[i] = Ovlxs[i].Ovl.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        //::_tprintf(_T("CreateEvent(): %ld\n"), ::GetLastError());
        ATLASSERT(hEvents[i] != NULL);

        ATLASSERT(::ConnectNamedPipe(Ovlxs[i].hPipe, (LPOVERLAPPED)&Ovlxs[i]) == 0);
        //::_tprintf(_T("ConnectNamedPipe(): %ld\n"), ::GetLastError());
        ATLASSERT(::GetLastError() == ERROR_IO_PENDING);
    }

    ::SetEvent(hEvent);

    TCHAR szBuf[BUF_SIZE];

    while (::_tcscmp(szBuf, _T("QUIT")))
    {
        //::_tprintf(_T("Connecting...\n"));
        DWORD dwIndex = ::WaitForMultipleObjects(PIPES, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
        //::_tprintf(_T("Connected.\n"));

        DWORD dwBytes = 0;
        BOOL bRes = ::GetOverlappedResult(Ovlxs[dwIndex].hPipe, (LPOVERLAPPED)&Ovlxs[dwIndex], &dwBytes, FALSE);

        bRes = ::ReadFile(Ovlxs[dwIndex].hPipe, szBuf, sizeof(szBuf), &dwBytes, NULL);
        if (bRes && dwBytes > 0)
            ::_tprintf(_T("[%s]\n"), szBuf);

        ATLASSERT(::DisconnectNamedPipe(Ovlxs[dwIndex].hPipe));

        ATLASSERT(::ConnectNamedPipe(Ovlxs[dwIndex].hPipe, (LPOVERLAPPED)&Ovlxs[dwIndex]) == 0);
        ATLASSERT(::GetLastError() == ERROR_IO_PENDING);
    }

    for (int i = 0; i < PIPES; i++)
    {
        ::DisconnectNamedPipe(Ovlxs[i].hPipe);
        ::CloseHandle(Ovlxs[i].Ovl.hEvent);
    }

    ::FreeSid(psid);
    ::LocalFree(pacl);
    ::LocalFree(psd);

    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
    {
        //TP_CALLBACK_ENVIRON cbe = { 0 };
        //::InitializeThreadpoolEnvironment(&cbe);

        //PTP_POOL ptpp = ::CreateThreadpool(NULL);
        //ATLASSERT(ptpp != NULL);

        //::SetThreadpoolThreadMaximum(ptpp, 16);
        //::SetThreadpoolCallbackPool(&cbe, ptpp);

        //PTP_CLEANUP_GROUP ptpcg = ::CreateThreadpoolCleanupGroup();
        //::SetThreadpoolCallbackCleanupGroup(&cbe, ptpcg, NULL);

        //PTP_WORK ptpw = ::CreateThreadpoolWork(&WorkCallback1, (PVOID)&cbe, &cbe);
        //::SubmitThreadpoolWork(ptpw);

        {
            CHandle hEvent = (CHandle)::CreateEvent(NULL, TRUE, FALSE, NULL);

            DWORD dwThreadId = 0;
            CHandle hThread = (CHandle)::CreateThread(NULL, 0, &ThreadProc, (PVOID)&hEvent, 0, &dwThreadId);

            ::WaitForSingleObject(hEvent, INFINITE);

            ::InstallHook();
            ::_getch();
            ::UninstallHook();

            CHandle hPipe = (CHandle)::CreateFile(
                NAMED_PIPE,
                GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);
            DWORD dwLastError = ::GetLastError();
            ATLASSERT(hPipe != INVALID_HANDLE_VALUE);

            DWORD dwMode = PIPE_READMODE_MESSAGE;
            ATLASSERT(::SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL));

            TCHAR szBuf[BUF_SIZE];
            DWORD dwBytes = 0;

            ::_stprintf_s(szBuf, _T("QUIT"));
            ::WriteFile(hPipe, szBuf, sizeof(szBuf), &dwBytes, NULL);

            ::FlushFileBuffers(hPipe);
            ::DisconnectNamedPipe(hPipe);

            ::WaitForSingleObject(hThread, INFINITE);
        }

        //::WaitForThreadpoolWorkCallbacks(ptpw, FALSE);

        //::CloseThreadpoolCleanupGroupMembers(ptpcg, FALSE, NULL);
        //::CloseThreadpoolCleanupGroup(ptpcg);
        //::CloseThreadpool(ptpp);
    }

    ::_getch();

    return 0;
}
