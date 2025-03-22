#include <iostream>
#include <windows.h>
#include <tlhelp32.h>


using namespace std;

void setColor(int Color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color);
}

DWORD getProcID(const wchar_t* procName)
{
	DWORD procID = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, procName))
				{
					procID = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
		CloseHandle(hSnap);
	}
	return procID;
}

bool InjectDLl(DWORD procID, const char* dllPath)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
	if (!hProcess) return false;

	LPVOID allocMem = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!allocMem) return false;

	WriteProcessMemory(hProcess, allocMem, dllPath, strlen(dllPath) + 1, NULL);
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, allocMem, 0, NULL);

	if (!hThread) return false;

	CloseHandle(hThread);
	CloseHandle(hProcess);
	return true;
}

int main()
{
	const wchar_t* gameName = L"GameName.exe";
	const char* dllPath = "PathToDLL";

	DWORD procID = getProcID(gameName);

	if (procID)
	{
		if (InjectDLl(procID, dllPath))
		{
			setColor(2);
			cout << "DLL Injected" << endl;
		}
		else
		{
			setColor(4);
			cout << "Failed To Inject DLL" << endl;
		}
	}
	else
	{
		setColor(4);
		cout << "Game Process Not Found" << endl;
	}

	system("pause");
	return 0;
}