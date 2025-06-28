#include <windows.h>
#include <iostream> 
#include "readmem.cpp"
#include <vector>




class FVP {
public:
    
    float screenWidth;
    float screenHeight;

    float playerFov = 90.0f;

    uintptr_t EntitylistPtr = 0x58AC04;
    uintptr_t PlayerCountAddr = 0x58AC0C;
    uintptr_t ViewMatrixAddr = 0x57DFD0;
    memory_manager memMgr;
    PIDGRAB PIDGRAB;

    DWORD pid = PIDGRAB.GetPIDByProcessName(L"ac_client.exe");
    HANDLE hProcess = memMgr.GetHandle(pid);


    float viewMatrix[16];
    float read = 0.0f;
    void SetScreenDimensions(float width, float height) {
        screenWidth = width;
        screenHeight = height;
    }
    std::vector<memory_manager::Player> loop() {

        int PlayerCount = memMgr.GrabIntMem(hProcess, PlayerCountAddr);


        uintptr_t EntityList = memMgr.ReadPtr(hProcess, EntitylistPtr);

                    //nutsack::localPlayerOffs localPlayerOffsets;
                    //nutsack::LocalPlayer localPlayer = NutSack.GetLocalPlayer(hProcess, localPlayerOffsets);
        
        
        std::vector<uintptr_t> handles;
        for (int i = 1; i < PlayerCount; i++) {
            uintptr_t CurrentEntityPtr = EntityList + i * 0x4;
            uintptr_t Out = memMgr.ReadPtr(hProcess, CurrentEntityPtr);
            handles.push_back(Out);
        }

        std::vector<memory_manager::Player> players;
        for (const auto& address : handles) {
            memory_manager::Player player = memMgr.GetPlayer(hProcess, address);
            players.push_back(player);
        }
        
        
        for (auto& player : players) {
                for (int iteration = 0; iteration < 16; iteration++) {

                ReadProcessMemory(hProcess, (LPCVOID)(ViewMatrixAddr + (iteration * 0x4)), &read, sizeof(read), NULL);
                viewMatrix[iteration] = read;
            }

            
            memory_manager::Vec3 head_pos = { player.x_pos, (player.y_pos + 5.0), player.z_pos };
            memory_manager::Vec3 foot_pos = { player.x_pos, player.y_pos, player.z_pos };
            memory_manager::Vec2 screen_head_pos = memMgr.OLDworld_to_screen(viewMatrix, head_pos, screenWidth, screenHeight);
            memory_manager::Vec2 screen_foot_pos = memMgr.OLDworld_to_screen(viewMatrix, foot_pos, screenWidth, screenHeight);

            player.head_screen_pos = screen_head_pos;
            player.feet_screen_pos = screen_foot_pos;


        }
        return players;
    };

};