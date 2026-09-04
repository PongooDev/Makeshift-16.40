// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

#include <stdarg.h>

/*-----------------------------------------------------------------------------
VarArgs helper macros.
-----------------------------------------------------------------------------*/

#define GET_VARARGS(msg, msgsize, len, lastarg, fmt) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		FCString::GetVarArgs(msg, msgsize, fmt, ap); \
		va_end(ap); \
	}
#define GET_VARARGS_WIDE(msg, msgsize, len, lastarg, fmt) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		FCStringWide::GetVarArgs(msg, msgsize, fmt, ap); \
		va_end(ap); \
	}
#define GET_VARARGS_ANSI(msg, msgsize, len, lastarg, fmt) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		FCStringAnsi::GetVarArgs(msg, msgsize, fmt, ap); \
		va_end(ap); \
	}
#define GET_VARARGS_RESULT(msg, msgsize, len, lastarg, fmt, result) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		result = FCString::GetVarArgs(msg, msgsize, fmt, ap); \
		if (result >= msgsize) \
		{ \
			result = -1; \
		} \
		va_end(ap); \
	}
#define GET_VARARGS_RESULT_WIDE(msg, msgsize, len, lastarg, fmt, result) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		result = FCStringWide::GetVarArgs(msg, msgsize, fmt, ap); \
		if (result >= msgsize) \
		{ \
			result = -1; \
		} \
		va_end(ap); \
	}
#define GET_VARARGS_RESULT_ANSI(msg, msgsize, len, lastarg, fmt, result) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		result = FCStringAnsi::GetVarArgs(msg, msgsize, fmt, ap); \
		if (result >= msgsize) \
		{ \
			result = -1; \
		} \
		va_end(ap); \
	}
