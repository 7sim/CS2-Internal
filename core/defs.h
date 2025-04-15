#pragma once
#include <Windows.h>
#include <chrono>

// access xorstr
#include "../deps/xorstr/xorstr.h"
#include <stb_image.h>
// using namespace stuff
using namespace std::chrono_literals;

// enable/disable console logging
//#ifdef _DEBUG
#define CS2_CONSOLE 0
//#endif

// replace this if you have a better way of doing it
#ifdef CS2_CONSOLE
#define CS2_LOG(...) printf_s(__VA_ARGS__)
#else
#define CS2_LOG(...)
#endif

// enable/disable unloading (press VK_END)
#ifdef _DEBUG
#define CS2_UNLOAD 1
#endif

// enable/disable sdk pointer printing
#ifdef _DEBUG
#define CS2_PTR_DEBUG 1
#define CS_LOG_CONSOLE 1
#define CS_LOG_FILE 0
#endif

#define STB_IMAGE_IMPLEMENTATION 1
#define STB_IMAGE_WRITE_IMPLEMENTATION 1