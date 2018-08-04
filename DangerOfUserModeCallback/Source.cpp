#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

typedef NTSTATUS(__stdcall *pfnNtUserSetWindowsHookEx)(HMODULE *hMod, UNICODE_STRING *pstrLib, UINT64 threadId, UINT64 nFilterType, HOOKPROC pfnProc, UINT64 dwFlags);
extern "C" NTSTATUS NtUserSetWindowsHookExGate(HMODULE *hMod, UNICODE_STRING *pstrLib, UINT64 threadId, UINT64 nFilterType, HOOKPROC pfnProc, UINT64 dwFlags);

HHOOK hHook;
LRESULT CALLBACK hookProc(int code, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(hHook, code, wParam, lParam);

}
DWORD WINAPI call_win32k(PVOID Parameter)
{
	printf("calling NtUserSetWindowsHookEx \n");
	NTSTATUS stat;
	HMODULE hMod = GetModuleHandle(NULL);
	pfnNtUserSetWindowsHookEx NtUserSetWindowsHookEx = (pfnNtUserSetWindowsHookEx)GetProcAddress(GetModuleHandle("win32u.dll"), "NtUserSetWindowsHookEx");

	stat = NtUserSetWindowsHookEx((HMODULE *)NULL, (UNICODE_STRING *)0xffff, 0, (UINT64)0, (HOOKPROC)0, 0);
	if (stat) {
		printf("can't set hook!");
		return EXIT_FAILURE;
	}

	return EXIT_FAILURE;
}

int main(int argc, char *argv[], char *env[])
{
	ULONG r = 0;

	HANDLE hThread = NULL;
	hThread = CreateThread(NULL, 0, call_win32k, NULL, 0, &r);
	if (hThread) {
		if (WaitForSingleObject(hThread, 20 * 1000) == WAIT_TIMEOUT) {
			TerminateThread(hThread, (DWORD)-1);
		}
		CloseHandle(hThread);
	}

	return 1;
}