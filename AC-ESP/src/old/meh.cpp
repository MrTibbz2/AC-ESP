#include <windows.h>
#include <iostream> 
#include "readmem.cpp"
#include <vector>
#include <imgui/imgui.h>
int main() {

    
    


    nutsack NutSack;
    PIDGRAB PIDGRAB;
    uintptr_t EntitylistPtr = 0x58AC04;
    uintptr_t PlayerCountAddr = 0x58AC0C;
    DWORD pid = PIDGRAB.GetPIDByProcessName(L"ac_client.exe");
    HANDLE hProcess = NutSack.GetHandle(pid);
    int PlayerCount = NutSack.GrabIntMem(hProcess, PlayerCountAddr);

    
    std::cout << PlayerCount << " was read" << std::endl;

    uintptr_t EntityList = NutSack.ReadPtr(hProcess, EntitylistPtr);

    std::vector<uintptr_t> handles;
    for (int i = 1; i < PlayerCount; i++) {
        uintptr_t CurrentEntityPtr = EntityList + i * 0x4;
        uintptr_t Out = NutSack.ReadPtr(hProcess, CurrentEntityPtr);
        handles.push_back(Out);
    }

    std::vector<nutsack::Player> players;
    for (const auto& address : handles) {
        nutsack::Player player = NutSack.GetPlayer(hProcess, address);
        players.push_back(player);
    }

    for (const auto& player : players) {
        std::cout << " ptr: " << std::hex << player.EntityPtr << std::dec << " Name: " << player.name << ", HP: " << player.health << " X: " << player.x_pos << " y: " << player.y_pos << " z: " << player.z_pos << std::endl;
    }


    return 0;
}