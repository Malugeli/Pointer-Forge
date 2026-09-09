#pragma once
#include <windows.h>
#include <utility>
#include <cmath>




struct ClientSide{// bisher sind wir nicht dynamisch. Wenn User bei Laufzeit die Resolution ändert ist ClientRect falsch.
    HWND game;
    RECT ClientRect;
    
    ClientSide() = default;
    ClientSide(HWND input) : game(input != NULL ? input : nullptr){
        GetClientRect(game, &ClientRect);
    }
    
    POINT normalize(POINT p){ // Konvertiert P zu SendInput fähigen Zahlen
        HMONITOR monitor = MonitorFromWindow(game, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info;
        info.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(monitor, &info);
        int width = info.rcMonitor.right - info.rcMonitor.left;
        int height = info.rcMonitor.bottom - info.rcMonitor.top;
        
        p.x = std::lround((p.x * 65535.0) / width);
        p.y = std::lround((p.y * 65535.0) / height);
        return p;
    }

    POINT convert_coordinates(POINT p){ // Konvertiert ClientToScreenPixel zu ScreenPixel und normalisiert für SendInput
        ClientToScreen(game, &p);
        return normalize(p);       
    }

    POINT get_UI_coordinates(UiTarget target){ // gib nur das UI ein und du erhältst SendInput Ready Koordinaten, dafür brauchst du ein Enum Class Array wie wir bei masterduel
        POINT p;
        p.x = std::lround(ClientRect.right * UI[std::to_underlying(target)].x);
        p.y = std::lround(ClientRect.bottom * UI[std::to_underlying(target)].y);
        return convert_coordinates(p);
    }
};

struct automate{ // Nur normalisierte Koordinaten angeben!
    INPUT inputM;
    INPUT inputK;

    //Maus
    void drag(POINT startcord, POINT targetcord){
    inputM.type = INPUT_MOUSE;
    inputM.mi.dx = startcord.x,
    inputM.mi.dy = startcord.y,
    inputM.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &inputM, sizeof(inputM));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    inputM.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &inputM, sizeof(inputM));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    inputM.mi.dx = targetcord.x;
    inputM.mi.dy = targetcord.y;
    inputM.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &inputM, sizeof(inputM));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    inputM.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &inputM, sizeof(inputM));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

    void click(POINT p){
        inputM.type = INPUT_MOUSE;
        inputM.mi.dx = p.x,
        inputM.mi.dy = p.y,
        inputM.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        SendInput(1, &inputM, sizeof(inputM));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        inputM.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &inputM, sizeof(inputM));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        inputM.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &inputM, sizeof(inputM));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

    //Tastatur
    void type_string(std::string_view s){
        inputK.type = INPUT_KEYBOARD;
            for(char c : s){
                SHORT checkKey = VkKeyScan(c);
                if((checkKey >> 8) & 1){
                    // Erstmal Shift drücken
                    inputK.ki.wScan = MapVirtualKey(VK_SHIFT, MAPVK_VK_TO_VSC);
                    inputK.ki.dwFlags = KEYEVENTF_SCANCODE;
                    SendInput(1, &inputK, sizeof(INPUT));
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); 

                    // Jetzt Buchstabe
                    BYTE virtualKey = VkKeyScan(c); // statt nochmal Funktionsaufruf kann ich Bitshiften und nur die untersten 8 Bits hier laden.
                    inputK.ki.wScan = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
                    inputK.ki.dwFlags = KEYEVENTF_SCANCODE;
                    SendInput(1, &inputK, sizeof(INPUT));
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); 

                    // Buchstabe Loslassen
                    inputK.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
                    SendInput(1, &inputK, sizeof(INPUT));   
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); 

                    // dann Shift loslassen
                    inputK.ki.wScan = MapVirtualKey(VK_SHIFT, MAPVK_VK_TO_VSC);
                    SendInput(1, &inputK, sizeof(INPUT));   
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
                }
                else
                {               
                    BYTE virtualKey = VkKeyScan(c);
                    inputK.ki.wScan = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
                    
                    inputK.ki.dwFlags = KEYEVENTF_SCANCODE; // Drücken (Key Down)

                    // --- KEY DOWN ---
                    SendInput(1, &inputK, sizeof(INPUT));
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); 

                    // --- KEY UP ---
                    inputK.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
                        // Loslassen
                    SendInput(1, &inputK, sizeof(INPUT));
                }
   }
}
};