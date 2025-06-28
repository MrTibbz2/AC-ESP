#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

#include <TlHelp32.h>
using namespace std;
	
class nutsack {
public:
	HANDLE GetHandle(DWORD pid) {
		// The process ID of our target process
	
	 // Prompting user for PID

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL) { // Failed to get a handle
		cout << "OpenProcess failed. GetLastError = " << dec << GetLastError() << endl;
		system("pause");
		return NULL;
	 



	}
	else {
		return hProcess;
	}



	}
	
	float GrabMem(HANDLE hProcess, uintptr_t MemAddr) {
		float ReadOut = 0;
		BOOL rpmReturn = ReadProcessMemory(hProcess, (LPCVOID)MemAddr, &ReadOut, sizeof(int), NULL);
		if (rpmReturn == FALSE) {
			cout << "ReadProcessMemory failed. GetLastError = " << dec << GetLastError() << endl;
			system("pause");
			return 0;
		}
		else {
			return ReadOut;
		}
	}
	
	int GrabIntMem(HANDLE hProcess, uintptr_t MemAddr) {
		int ReadOut = 0;
		BOOL rpmReturn = ReadProcessMemory(hProcess, (LPCVOID)MemAddr, &ReadOut, sizeof(int), NULL);
		if (rpmReturn == FALSE) {
			cout << "ReadProcessMemory failed. GetLastError = " << dec << GetLastError() << endl;
			system("pause");
			return 0;
		}
		else {
			return ReadOut;
		}
	}
	
	uintptr_t ReadPtr(HANDLE hProcess, uintptr_t MemAddr) {
		uintptr_t ReadOut = 0x0;
		BOOL rpmReturn = ReadProcessMemory(hProcess, (LPCVOID)MemAddr, &ReadOut, sizeof(int), NULL);
		if (rpmReturn == FALSE) {
			cout << "ReadProcessMemory failed. GetLastError = " << dec << GetLastError() << endl;
			system("pause");
			return 0;
		}
		else {
			return ReadOut;
		}
	}

	struct Addresses {
		vector<uintptr_t> handles;
	};
	
	Addresses EnumPlayers(HANDLE hProcess, int PlayerCount, uintptr_t EntityList) {
		Addresses addresses;
		for (int i = 1; i < PlayerCount; i++) {
			
			uintptr_t CurrentEntityPtr = EntityList + i * 0x4;
			uintptr_t Out = ReadPtr(hProcess, CurrentEntityPtr);

			addresses.handles.push_back(Out);
			
		}
		return addresses;
	}
	
	struct Player {
		uintptr_t EntityPtr = 0x0;
		char name[256] = "hi";
		int health = 0;
		float x_pos = 0.0f;
		float y_pos = 0.0f;
		float z_pos = 0.0f;
	};

	Player GetPlayer(HANDLE hProcess, uintptr_t EntityPtr) {
		Player player;
		player.EntityPtr = EntityPtr;
		uintptr_t NamePtr = EntityPtr + 0x205;
		uintptr_t HealthPtr = EntityPtr + 0xEC;
		uintptr_t XPosPtr = EntityPtr + 0x2C;
		uintptr_t YPosPtr = EntityPtr + 0x30;
		uintptr_t ZPosPtr = EntityPtr + 0x28;

		char Name[16] = { 0 }; // Ensure the string is zero-terminated

		ReadProcessMemory(hProcess, (LPCVOID)NamePtr, &Name, sizeof(Name), NULL);

		strcpy_s(player.name, Name); // Copy the name to the player's name array

		player.health = GrabIntMem(hProcess, HealthPtr);
		player.x_pos = GrabMem(hProcess, XPosPtr);
		player.y_pos = GrabMem(hProcess, YPosPtr);
		player.z_pos = GrabMem(hProcess, ZPosPtr);

		return player;
	}
};
	
class PIDGRAB {
public:
	DWORD GetPIDByProcessName(const wchar_t* processName) {
		DWORD PID = 0;
		HANDLE hProcessSnapshot;
		PROCESSENTRY32 PE32;

		// Take a snapshot of all processes in the system.
		hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (hProcessSnapshot == INVALID_HANDLE_VALUE)
		{
			std::cout << "<CreateToolhelp32Snapshot> Invalid handle";
			return 0;
		}

		// Set the size of the structure before using it.
		PE32.dwSize = sizeof(PROCESSENTRY32);

		// Retrieves information about the first process and exit if unsuccessful
		if (!Process32First(hProcessSnapshot, &PE32))
		{
			std::cout << "<Process32First> Error " << GetLastError() << '\n';
			CloseHandle(hProcessSnapshot);
			return 0;
		}

		// Now walk the snapshot of processes,
		// and find the right process then get its PID
		do
		{
			// Returns 0 value indicates that both wchar_t* string are equal
			if (wcscmp(processName, PE32.szExeFile) == 0)
			{
				PID = PE32.th32ProcessID;
				break;
			}
		} while (Process32Next(hProcessSnapshot, &PE32));

		CloseHandle(hProcessSnapshot);
		return PID;
	}

};