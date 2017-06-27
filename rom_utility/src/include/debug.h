
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "io.h"

#define ROM_TOOL_DEBUG

#ifdef ROM_TOOL_DEBUG
#define DEBUG_LOG printf
#else
#define DEBUG_LOG(...)
#endif

#endif

