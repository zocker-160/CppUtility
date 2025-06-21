/*
 * Helper.cpp
 *
 * @author zocker_160
 * @license MIT
 * @description collection of useful functions for DLL hacks
 */

#include <iostream>
#include <fstream>

#include "Helper.h"
#include "SHA256.h"

 // reading and writing stuff / helper functions and other crap

 /* update memory protection and read with memcpy */
void protectedRead(void* dest, void* src, int n) {
    DWORD oldProtect = 0;
    VirtualProtect(dest, n, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(dest, src, n);
    VirtualProtect(dest, n, oldProtect, &oldProtect);
}
/* read from address into read buffer of length len */
bool readBytes(void* read_addr, void* read_buffer, int len) {
    // compile with "/EHa" to make this work
    // see https://stackoverflow.com/questions/16612444/catch-a-memory-access-violation-in-c
    try {
        protectedRead(read_buffer, read_addr, len);
        return true;
    }
    catch (...) {
        return false;
    }
}
/* write patch of length len to destination address */
void writeBytes(void* dest_addr, void* patch, int len) {
    protectedRead(dest_addr, patch, len);
}

/* check if specific memory address is readable by us */
bool isMemoryReadable(void* addr) {
    MEMORY_BASIC_INFORMATION mbi = {0};
    if (VirtualQuery(addr, &mbi, sizeof(mbi)) == 0)
        return false;                         // address outside any region

    if (mbi.State != MEM_COMMIT)             // reserved or free
        return false;

    // Strip out the modifier bits (PAGE_GUARD, PAGE_NOCACHE, etc.)
    DWORD prot = mbi.Protect & 0xFF;

    switch (prot) {
        case PAGE_READONLY:
        case PAGE_READWRITE:
        case PAGE_WRITECOPY:
        case PAGE_EXECUTE_READ:
        case PAGE_EXECUTE_READWRITE:
        case PAGE_EXECUTE_WRITECOPY:
            return true;                     // page allows reads
        default:
            return false;                    // PAGE_NOACCESS, PAGE_EXECUTE, …
    }
}

/* fiddle around with the pointers */
HMODULE getBaseAddress() {
    return GetModuleHandleA(NULL);
}
HMODULE getModuleAddress(LPCSTR moduleName) {
    return GetModuleHandleA(moduleName);
}

DWORD* calcAddress(DWORD appl_addr) {
    return (DWORD*)((DWORD)getBaseAddress() + appl_addr);
}
DWORD* calcModuleAddress(HMODULE module, DWORD appl_addr) {
    return (DWORD*)((DWORD)module + appl_addr);
}
DWORD* tracePointer(memoryPTR* patch) {
    DWORD* location = calcAddress(patch->base_address);

    for (int n : patch->offsets) {
        location = (DWORD*)(*location + n);
    }

    return location;
}

void nopper(void* startAddr, int len) {
    BYTE nop = 0x90;
    for (int i = 0; i < len; i++)
        writeBytes((DWORD*)((DWORD)startAddr + i), &nop, 1);
}

bool functionInjector(void* hookAddr, void* function, int len) {
    if (len < 5)
        return false;

    BYTE jmp = 0xE9;
    DWORD relAddr = ((DWORD)function - (DWORD)hookAddr) - 5;

    /* NOP needed area */
    nopper(hookAddr, len);

    writeBytes(hookAddr, &jmp, 1);
    writeBytes((DWORD*)((DWORD)hookAddr + 1), &relAddr, 4);

    return true;
}
bool functionInjectorReturn(void* hookAddr, void* function, DWORD& returnAddr, int len) {
    if (len < 5)
        return false;

    BYTE jmp = 0xE9;
    DWORD relAddr = ((DWORD)function - (DWORD)hookAddr) - 5;
    returnAddr = (DWORD)hookAddr + len;

    nopper(hookAddr, len);

    writeBytes(hookAddr, &jmp, 1);
    writeBytes((DWORD*)((DWORD)hookAddr + 1), &relAddr, 4);

    return true;
}

/* resolution stuff */

void getDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
}
void getDesktopResolution2(int& hor, int& vert) {
    hor = GetSystemMetrics(SM_CXSCREEN);
    vert = GetSystemMetrics(SM_CYSCREEN);
}

void getMainScreenResolution(int& hor, int& vert) {
    DEVMODE lpDevMode;
    memset(&lpDevMode, 0, sizeof(lpDevMode));
    lpDevMode.dmSize = sizeof(DEVMODE);
    lpDevMode.dmDriverExtra = 0;

    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode)) {
        hor = lpDevMode.dmPelsWidth;
        vert = lpDevMode.dmPelsHeight;
    }
}

int getDesktopRefreshRate() {
    DEVMODE lpDevMode;
    memset(&lpDevMode, 0, sizeof(lpDevMode));
    lpDevMode.dmSize = sizeof(DEVMODE);
    lpDevMode.dmDriverExtra = 0;

    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode))
        return lpDevMode.dmDisplayFrequency;
    else
        return 0;
}

float getAspectRatio() {
    int horizontal, vertical;

    getDesktopResolution2(horizontal, vertical);
    return calcAspectRatio(horizontal, vertical);
}

float calcAspectRatio(int horizontal, int vertical) {
    if (horizontal > 0 && vertical > 0)
        return (float)horizontal / (float)vertical;
    else
        return -1.0f;
}

/* other helper functions and stuff */
bool isKeyPressed(int vKey) {
    /* some bitmask trickery because why not */
    return GetAsyncKeyState(vKey) & 0x8000;
}

bool isWine() {
    HMODULE ntdllMod = GetModuleHandleA("ntdll.dll");

    return ntdllMod && GetProcAddress(ntdllMod, "wine_get_version");
}

bool isVulkanSupported() {
    HMODULE vk = LoadLibraryA("vulkan-1.dll");
    if (!vk)
        return false; // Vulkan drivers not installed

    typedef VOID*(WINAPI* VK_GETINSTANCE_PROC)(void*, const char*);
    typedef INT(WINAPI* VK_CREATE_INSTANCE)(int*, void*, void**);

    auto vkGetInstanceProcAddr = (VK_GETINSTANCE_PROC)GetProcAddress(vk, "vkGetInstanceProcAddr");
    if (!vkGetInstanceProcAddr)
        return false; // Vulkan is malfunctioning (vkGetInstanceProcAddr)

    auto vkCreateInstance = (VK_CREATE_INSTANCE)vkGetInstanceProcAddr(0, "vkCreateInstance");
    if (!vkCreateInstance)
        return false; // Vulkan is malfunctioning (vkCreateInstance)

    void* instance = 0;
    int dummyInt[16] = { 1 };
    int result = vkCreateInstance(dummyInt, 0, &instance);

    if (!instance || result != 0)
        return false; // Vulkan drivers installed and functioning, but are incompatible. An instance could not be created

    return true;
}

HMODULE getBaseModule() {
    HMODULE base;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)getBaseAddress(),
        &base
    );
    return base;
}

void getGameDirectory(HMODULE hm, char* path, int size, const char* loc, int levels) {
    GetModuleFileNameA(hm, path, size);

    for (int i = 0; i <= levels; i++) {
        *strrchr(path, '\\') = '\0';
    }

    strcat_s(path, size, loc);
}


bool getFileChecksum(char* filePath, std::string& checksum) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }

    size_t filesize = file.tellg();
    char* filebuffer = (char*)malloc(filesize);

    file.seekg(0, std::ios::beg);
    file.read(filebuffer, filesize);
    file.close();

    checksum = sha256(filebuffer, filesize);

    free(filebuffer);

    return true;
}
