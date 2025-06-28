#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <iostream>
#include <string>
#include <vector>
#include <TlHelp32.h>
#include <algorithm>

using namespace std;


class memory_manager {
public:
    struct Vector4 {
        float x, y, z, w;
    };
    struct Vec3 {
        float x, y, z;
    };

    struct Vec2 {
        float x, y;
        
        
    };
    struct Vec4 {
        float x, y, z, w;
    };

    HANDLE GetHandle(DWORD pid) {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (hProcess == NULL) {
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
        Vec2 head_screen_pos = { -1, -1 };
        Vec2 feet_screen_pos = { -1, -1 };
    };

    Player GetPlayer(HANDLE hProcess, uintptr_t EntityPtr) {
        Player player;
        player.EntityPtr = EntityPtr;
        uintptr_t NamePtr = EntityPtr + 0x205;
        uintptr_t HealthPtr = EntityPtr + 0xEC;
        uintptr_t XPosPtr = EntityPtr + 0x2C;
        uintptr_t YPosPtr = EntityPtr + 0x30;
        uintptr_t ZPosPtr = EntityPtr + 0x28;

        char Name[16] = { 0 };

        ReadProcessMemory(hProcess, (LPCVOID)NamePtr, &Name, sizeof(Name), NULL);
        strcpy_s(player.name, Name);

        player.health = GrabIntMem(hProcess, HealthPtr);
        player.x_pos = GrabMem(hProcess, XPosPtr);
        player.y_pos = GrabMem(hProcess, YPosPtr);
        player.z_pos = GrabMem(hProcess, ZPosPtr);

        return player;
    }

    struct localPlayerOffs {
        uintptr_t LocalPlayer = 0x00775A08;
        uintptr_t XPosPtr = LocalPlayer + 0x2C;
        uintptr_t YPosPtr = LocalPlayer + 0x30;
        uintptr_t ZPosPtr = LocalPlayer + 0x28;
        uintptr_t CameraX = LocalPlayer + 0x34;
        uintptr_t CameraY = LocalPlayer + 0x38;
    };

    struct LocalPlayer {
        float x_pos = 0.0f;
        float y_pos = 0.0f;
        float z_pos = 0.0f;
        float camera_x = 0.0f;
        float camera_y = 0.0f;
    };

    LocalPlayer GetLocalPlayer(HANDLE hProcess, localPlayerOffs offs) {
        LocalPlayer localPlayer;
        localPlayer.x_pos = GrabMem(hProcess, offs.XPosPtr);
        localPlayer.y_pos = GrabMem(hProcess, offs.YPosPtr);
        localPlayer.z_pos = GrabMem(hProcess, offs.ZPosPtr);
        localPlayer.camera_x = GrabMem(hProcess, offs.CameraX);
        localPlayer.camera_y = GrabMem(hProcess, offs.CameraY);

        return localPlayer;
    }
    
    

    
    Vec2 OLDworld_to_screen(const float matrix[16], const Vec3 pos, float screen_width, float screen_height) {
        Vec3 clip;
        Vec2 ndc;
        Vec3 ActPos;
        ActPos.y = pos.x;
        ActPos.x = pos.z;
        ActPos.z = pos.y;
        

        clip.z = ActPos.x * matrix[3] + ActPos.y * matrix[7] + ActPos.z * matrix[11] + matrix[15];


        if (clip.z < 0.2) {  // Adjusted threshold for clipping
            Vec2 beans;
            beans.x = -1;
            beans.y = -1;
            return beans;
        }

        clip.x = ActPos.x * matrix[0] + ActPos.y * matrix[4] + ActPos.z * matrix[8] + matrix[12];
        clip.y = ActPos.x * matrix[1] + ActPos.y * matrix[5] + ActPos.z * matrix[9] + matrix[13];
        ndc.x = clip.x / clip.z;
        ndc.y = clip.y / clip.z;

        // Convert NDC to screen space and clamp values to ensure they're within screen bounds
        float result_x = (ndc.x + 1) * (screen_width / 2);
        float result_y = (1 - ndc.y) * (screen_height / 2);

        if (result_x > screen_width or result_x < 0) {
			result_x = NULL;
		}
        if (result_y > screen_height or result_y < 0) {
            result_y = NULL;
        }

        return { result_x, result_y };
    }
    
}; // Ensure the class is properly closed with a semicolon

class PIDGRAB {
public:
    DWORD GetPIDByProcessName(const wchar_t* processName) {
        DWORD PID = 0;
        HANDLE hProcessSnapshot;
        PROCESSENTRY32 PE32;

        hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (hProcessSnapshot == INVALID_HANDLE_VALUE) {
            std::cout << "<CreateToolhelp32Snapshot> Invalid handle";
            return 0;
        }

        PE32.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(hProcessSnapshot, &PE32)) {
            std::cout << "<Process32First> Error " << GetLastError() << '\n';
            CloseHandle(hProcessSnapshot);
            return 0;
        }

        do {
            if (wcscmp(processName, PE32.szExeFile) == 0) {
                PID = PE32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnapshot, &PE32));

        CloseHandle(hProcessSnapshot);
        return PID;
    }
}; // Ensure the class is properly closed with a semicolon
