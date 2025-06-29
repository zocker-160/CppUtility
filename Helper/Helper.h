/*
 * Helper.h
 *
 * @author zocker_160
 * @license MIT
 * @description collection of useful functions for DLL hacks
 */
#pragma once

#include <windows.h>

#include <vector>
#include <sstream>
#include <iostream>

#define NAKED __declspec(naked) void
#define JMP(addr) {_asm{jmp[addr]}}
#define DLLEXPORT(ddraw, func) NAKED f_##func() JMP(ddraw.func)
#define PROXYADDR(ddraw, module, func) ddraw.func = GetProcAddress(module, #func)


struct memoryPTR {
    DWORD base_address;
    std::vector<int> offsets;
};

void protectedRead(void* dest, void* src, int n);
bool readBytes(void* read_addr, void* read_buffer, int len);
void writeBytes(void* dest_addr, void* patch, int len);

bool isMemoryReadable(void* addr);

HMODULE getBaseAddress();
HMODULE getModuleAddress(LPCSTR moduleName);

DWORD* calcAddress(DWORD appl_addr);
DWORD* calcModuleAddress(HMODULE module, DWORD appl_addr);
DWORD* tracePointer(memoryPTR* patch);

void nopper(void* startAddr, int len);
bool functionInjector(void* hookAddr, void* function, int len);
bool functionInjectorReturn(void* hookAddr, void* function, DWORD& returnAddr, int len);

void getDesktopResolution(int& horizontal, int& vertical);
void getDesktopResolution2(int& hor, int& vert);
void getMainScreenResolution(int& hor, int& vert);

int getDesktopRefreshRate();

HMODULE getBaseModule();
void getGameDirectory(HMODULE hm, char* path, int size, const char* location, int levels = 0);

bool isKeyPressed(int vKey);

bool isWine();
bool isVulkanSupported();

float getAspectRatio();
float calcAspectRatio(int horizontal, int vertical);

bool getFileChecksum(char* filePath, std::string& checksum);
