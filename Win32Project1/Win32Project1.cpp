// Win32Project1.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <Psapi.h>
#pragma comment(lib, "Psapi.Lib")

#include <Sddl.h>

extern HMODULE g_hModule;

#define IDM_LEFTHALF                0x0010
#define IDM_RIGHTHALF               0x0011
#define IDM_BOTTOMTHIRD             0x0012
#define IDM_REMEMBER                0x0013

enum HOOKID
{
    CBT = 0,
    SHELL,
    GETMESSAGE
};

HHOOK g_hHook[3];
BOOL g_bError = FALSE;

void WritePipe(LPCTSTR psz)
{
    //if (g_bError)
    //    return;

    CHandle hPipe = (CHandle)::CreateFile(
        _T("\\\\.\\pipe\\F09DC44C-33F3-432D-BE51-FF56194F062A"),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    DWORD dwError = ::GetLastError();
    //ATLASSERT(hPipe != INVALID_HANDLE_VALUE);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        g_bError = TRUE;

        //CString szUserName, szDomainName;

        //{
        //    HANDLE hToken;
        //    if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken))
        //    {
        //        szUserName = _T("OpenProcessToken()");

        //        DWORD dwBytes = 0;
        //        if (!::GetTokenInformation(hToken, TokenUser, NULL, 0, &dwBytes))
        //        {
        //            dwError = ::GetLastError();
        //            if (dwError == ERROR_INSUFFICIENT_BUFFER)
        //            {
        //                szUserName = _T("GetTokenInformation() [1]");

        //                PTOKEN_USER ptu = (PTOKEN_USER)::GlobalAlloc(GPTR, dwBytes);
        //                if (ptu)
        //                {
        //                    szUserName = _T("GlobalAlloc()");

        //                    if (::GetTokenInformation(hToken, TokenUser, ptu, dwBytes, &dwBytes))
        //                    {
        //                        szUserName = _T("GetTokenInformation() [2]");

        //                        DWORD dwUserNameBytes = 256, dwDomainNameBytes = 256;
        //                        SID_NAME_USE snu;
        //                        if (::LookupAccountSid(
        //                            NULL,
        //                            ptu->User.Sid,
        //                            CStrBuf(szUserName, 256),
        //                            &dwUserNameBytes,
        //                            CStrBuf(szDomainName, 256),
        //                            &dwDomainNameBytes,
        //                            &snu))
        //                        {
        //                        }

        //                        //LPTSTR psid = NULL;
        //                        //if (::ConvertSidToStringSid(ptu->User.Sid, &psid))
        //                        //{
        //                        //    ::LocalFree(psid);
        //                        //    psid = NULL;
        //                        //}
        //                    }

        //                    ::GlobalFree(ptu);
        //                    ptu = NULL;
        //                }
        //            }
        //        }

        //        //dwError = ::GetLastError();

        //        ::CloseHandle(hToken);
        //        hToken = INVALID_HANDLE_VALUE;
        //    }
        //}

        //CString sz;
        //sz.Format(_T("CreateFile(): %ld"), dwError);
        //::MessageBox(NULL, (LPCTSTR)sz, _T("Win32ConsoleApplication3"), MB_OK | MB_ICONERROR);
    }

    DWORD dwMode = PIPE_READMODE_MESSAGE;
    ::SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);

    CString szProcessName;
    ::GetProcessImageFileName(::GetCurrentProcess(), CStrBuf(szProcessName, 65535), 65535);

    CString szModuleName;
    ::GetModuleFileName(NULL, CStrBuf(szModuleName, 65535), 65535);

    CString sz;
    sz.Format(_T("[%s][%s]: %s"), szProcessName, szModuleName, psz);

    TCHAR szBuf[512];
    DWORD dwBytes = 0;

    ::_stprintf_s(szBuf, sz);
    ::WriteFile(hPipe, szBuf, sizeof(szBuf), &dwBytes, NULL);

    ::FlushFileBuffers(hPipe);
    ::DisconnectNamedPipe(hPipe);
}

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
        return ::CallNextHookEx(g_hHook[CBT], nCode, wParam, lParam);

    if (nCode == HC_ACTION)
    {
        BOOL bMsgSentByCurrentThread = (BOOL)wParam;
        PCWPSTRUCT p = (PCWPSTRUCT)lParam;

        //TCHAR szBuf[512];
        //::_stprintf_s(szBuf, _T("CBTProc(), nCode = %ld, p->message = 0x%X"), nCode, p->message);
        //::WritePipe(szBuf);
    }

    return ::CallNextHookEx(g_hHook[CBT], nCode, wParam, lParam);
}

LRESULT CALLBACK ShellProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
        return ::CallNextHookEx(g_hHook[CBT], nCode, wParam, lParam);

    if (nCode == HSHELL_WINDOWCREATED
        || nCode == HSHELL_WINDOWDESTROYED)
    {
        if (nCode == HSHELL_WINDOWCREATED)
        {
            HWND hWnd = (HWND)wParam;

            CString szWindowText;
            ::GetWindowText(hWnd, CStrBuf(szWindowText, 256), 256);

            RECT rc = { 0 };
            ATLASSERT(::GetWindowRect(hWnd, &rc));

            //{
            //    CString sz;
            //    sz.Format(_T("ShellProc(), nCode = HSHELL_WINDOWCREATED, sz = [%s], left:%ld, top:%ld, right:%ld, bottom:%ld"), (LPCTSTR)szWindowText, rc.left, rc.top, rc.right, rc.bottom);
            //    ::WritePipe(sz);
            //}

            HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
            ATLASSERT(hMonitor != NULL);

            MONITORINFO mi = { 0 };
            mi.cbSize = sizeof(MONITORINFO);
            ATLASSERT(::GetMonitorInfo(hMonitor, &mi));

            HMENU hMenu = ::GetSystemMenu(hWnd, FALSE);
            ATLASSERT(hMenu != NULL);

            //MENUITEMINFO mii = { 0 };
            //mii.cbSize = sizeof(MENUITEMINFO);
            //mii.fMask = MIIM_STRING;
            //mii.dwTypeData = _T("TEST");
            //mii.cch = 4;
            //mii.fType = MFT_STRING;
            //mii.wID = 100;
            //::InsertMenuItem(hMenu, 0, TRUE, &mii);

            HMENU hSubMenu = ::CreatePopupMenu();

            ::InsertMenu(hSubMenu, 0, MF_BYPOSITION | MF_STRING, IDM_LEFTHALF, _T("&Left Half"));
            ::InsertMenu(hSubMenu, 1, MF_BYPOSITION | MF_STRING, IDM_RIGHTHALF, _T("&Right Half"));
            ::InsertMenu(hSubMenu, 2, MF_BYPOSITION | MF_STRING, IDM_BOTTOMTHIRD, _T("&Bottom Third"));
            ::InsertMenu(hSubMenu, 3, MF_BYPOSITION | MF_STRING, IDM_REMEMBER, _T("Re&member"));

            ::InsertMenu(hMenu, 0, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hSubMenu, _T("Position to"));
            ::InsertMenu(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);

            //{
            //    CString sz;
            //    sz.Format(_T("%ld, x:%ld, y:%ld, x:%ld, y:%ld"), ::GetLastError(), mi.rcWork.left, mi.rcWork.top, mi.rcWork.right, mi.rcWork.bottom);
            //    ::WritePipe(sz);
            //}

            if (!::_tcscmp(szWindowText, _T("Run")))
            {
                // Bottom left
                ::SetWindowPos(
                    hWnd,
                    HWND_TOP,
                    0,
                    mi.rcWork.bottom - (rc.bottom - rc.top),
                    rc.right - rc.left,
                    rc.bottom - rc.top,
                    0);

                // Bottom right
                //::SetWindowPos(
                //    hWnd,
                //    HWND_TOP,
                //    mi.rcWork.right - (rc.right - rc.left),
                //    mi.rcWork.bottom - (rc.bottom - rc.top),
                //    rc.right - rc.left,
                //    rc.bottom - rc.top,
                //    0);
            }
            //else if (!::_tcscmp(szWindowText, _T("Computer")))
            //{
            //    ::SetWindowPos(
            //        hWnd,
            //        HWND_TOP,
            //        mi.rcWork.right - 1800,
            //        0,
            //        1800,
            //        mi.rcWork.bottom,
            //        0);
            //}
            //else if (!::_tcscmp(szWindowText, _T("C:\\Windows\\system32\\cmd.exe")))
            //{
            //    ::SetWindowPos(
            //        hWnd,
            //        HWND_TOP,
            //        0,
            //        0,
            //        mi.rcWork.right,
            //        mi.rcWork.bottom,
            //        0);

            //    //ATLASSERT(::GetWindowRect(hWnd, &rc));

            //    //{
            //    //    CString sz;
            //    //    sz.Format(_T("left:%ld, top:%ld, right:%ld, bottom:%ld"), rc.left, rc.top, rc.right, rc.bottom);
            //    //    ::WritePipe(sz);
            //    //}
            //}
            //else if(!::_tcscmp(szWindowText, _T("Windows Internet Explorer")))
            //{
            //    ::SetWindowPos(
            //        hWnd,
            //        HWND_TOP,
            //        mi.rcWork.right - 1800,
            //        0,
            //        mi.rcWork.right - (mi.rcWork.right - 1800),
            //        mi.rcWork.bottom,
            //        0);
            //}
        }
        //else if (nCode == HSHELL_WINDOWDESTROYED)
        //{
        //    HWND hWnd = (HWND)wParam;

        //    CString szWindowText;
        //    ::GetWindowText(hWnd, CStrBuf(szWindowText, 256), 256);

        //    {
        //        CString sz;
        //        sz.Format(_T("ShellProc(), nCode = HSHELL_WINDOWDESTROYED, sz = [%s]"), (LPCTSTR)szWindowText);
        //        ::WritePipe(sz);
        //    }
        //}
    }

    return ::CallNextHookEx(g_hHook[CBT], nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
        return ::CallNextHookEx(g_hHook[GETMESSAGE], nCode, wParam, lParam);

    if (wParam == PM_REMOVE)
    {
        PMSG pMsg = (PMSG)lParam;

        if (pMsg->message == WM_SYSCOMMAND || pMsg->message == 0x80F0)
        {
            HMONITOR hMonitor = ::MonitorFromWindow(pMsg->hwnd, MONITOR_DEFAULTTONEAREST);
            ATLASSERT(hMonitor != NULL);

            MONITORINFO mi = { 0 };
            mi.cbSize = sizeof(MONITORINFO);
            ATLASSERT(::GetMonitorInfo(hMonitor, &mi));

            switch (LOWORD(pMsg->wParam))
            {
                case IDM_LEFTHALF:
                    {
                        //CString sz;
                        //sz.Format(_T("GetMsgProc(), IDM_LEFTHALF"));
                        //::WritePipe(sz);

                        ::SetWindowPos(
                            pMsg->hwnd,
                            HWND_TOP,
                            0,
                            0,
                            mi.rcWork.right / 2,
                            mi.rcWork.bottom,
                            0);
                    }
                    break;
                case IDM_RIGHTHALF:
                    {
                        //CString sz;
                        //sz.Format(_T("GetMsgProc(), IDM_RIGHTHALF"));
                        //::WritePipe(sz);

                        ::SetWindowPos(
                            pMsg->hwnd,
                            HWND_TOP,
                            mi.rcWork.right / 2,
                            0,
                            mi.rcWork.right - (mi.rcWork.right - (mi.rcWork.right / 2)),
                            mi.rcWork.bottom,
                            0);
                    }
                    break;
                case IDM_BOTTOMTHIRD:
                    {
                        //CString sz;
                        //sz.Format(_T("GetMsgProc(), IDM_BOTTOMTHIRD"));
                        //::WritePipe(sz);

                        ::SetWindowPos(
                            pMsg->hwnd,
                            HWND_TOP,
                            0,
                            mi.rcWork.bottom - (mi.rcWork.bottom / 3),
                            mi.rcWork.right,
                            mi.rcWork.bottom - (mi.rcWork.bottom - ((mi.rcWork.bottom / 3))),
                            0);
                    }
                    break;
                case IDM_REMEMBER:
                    {
                        //CString sz;
                        //sz.Format(_T("GetMsgProc(), IDM_REMEMBER"));
                        //::WritePipe(sz);
                    }
                    break;
                default:
                    break;
            }

        }
    }

    return ::CallNextHookEx(g_hHook[GETMESSAGE], nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) DWORD InstallHook()
{
    g_hHook[CBT] = ::SetWindowsHookEx(WH_CBT, &CBTProc, g_hModule, 0);
    g_hHook[SHELL] = ::SetWindowsHookEx(WH_SHELL, &ShellProc, g_hModule, 0);
    g_hHook[GETMESSAGE] = ::SetWindowsHookEx(WH_GETMESSAGE, &GetMsgProc, g_hModule, 0);

    return 0;
}

extern "C" __declspec(dllexport) DWORD UninstallHook()
{
    ::UnhookWindowsHookEx(g_hHook[CBT]);
    ::UnhookWindowsHookEx(g_hHook[SHELL]);
    ::UnhookWindowsHookEx(g_hHook[GETMESSAGE]);

    return 0;
}
