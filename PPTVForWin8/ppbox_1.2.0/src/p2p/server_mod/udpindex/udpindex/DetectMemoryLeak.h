#pragma once
#ifdef _DEBUG
#define DEBUG_CLIENTBLOCK new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_CLIENTBLOCK
#endif

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_CLIENTBLOCK
#endif