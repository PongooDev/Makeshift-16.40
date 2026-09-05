// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include <intrin.h>

/**
 * Windows implementation of the Atomics OS functions
 */
struct CORE_API FWindowsPlatformAtomics
{
	static_assert(sizeof(int8)  == sizeof(char)      && alignof(int8)  == alignof(char),      "int8 must be compatible with char");
	static_assert(sizeof(int16) == sizeof(short)     && alignof(int16) == alignof(short),     "int16 must be compatible with short");
	static_assert(sizeof(int32) == sizeof(long)      && alignof(int32) == alignof(long),      "int32 must be compatible with long");
	static_assert(sizeof(int64) == sizeof(long long) && alignof(int64) == alignof(long long), "int64 must be compatible with long long");

	static FORCEINLINE int8 InterlockedIncrement( volatile int8* Value )
	{
		return (int8)::_InterlockedExchangeAdd8((char*)Value, 1) + 1;
	}

	static FORCEINLINE int16 InterlockedIncrement( volatile int16* Value )
	{
		return (int16)::_InterlockedIncrement16((short*)Value);
	}

	static FORCEINLINE int32 InterlockedIncrement( volatile int32* Value )
	{
		return (int32)::_InterlockedIncrement((long*)Value);
	}

	static FORCEINLINE int64 InterlockedIncrement( volatile int64* Value )
	{
		#if PLATFORM_64BITS
			return (int64)::_InterlockedIncrement64((long long*)Value);
		#else
			// No explicit instruction for 64-bit atomic increment on 32-bit processors; has to be implemented in terms of CMPXCHG8B
			for (;;)
			{
				int64 OldValue = *Value;
				if (_InterlockedCompareExchange64(Value, OldValue + 1, OldValue) == OldValue)
				{
					return OldValue + 1;
				}
			}
		#endif
	}

	static FORCEINLINE int8 InterlockedDecrement( volatile int8* Value )
	{
		return (int8)::_InterlockedExchangeAdd8((char*)Value, -1) - 1;
	}

	static FORCEINLINE int16 InterlockedDecrement( volatile int16* Value )
	{
		return (int16)::_InterlockedDecrement16((short*)Value);
	}

	static FORCEINLINE int32 InterlockedDecrement( volatile int32* Value )
	{
		return (int32)::_InterlockedDecrement((long*)Value);
	}

	static FORCEINLINE int64 InterlockedDecrement( volatile int64* Value )
	{
		#if PLATFORM_64BITS
			return (int64)::_InterlockedDecrement64((long long*)Value);
		#else
			// No explicit instruction for 64-bit atomic decrement on 32-bit processors; has to be implemented in terms of CMPXCHG8B
			for (;;)
			{
				int64 OldValue = *Value;
				if (_InterlockedCompareExchange64(Value, OldValue - 1, OldValue) == OldValue)
				{
					return OldValue - 1;
				}
			}
		#endif
	}

	static FORCEINLINE int8 InterlockedAdd( volatile int8* Value, int8 Amount )
	{
		return (int8)::_InterlockedExchangeAdd8((char*)Value, (char)Amount);
	}

	static FORCEINLINE int16 InterlockedAdd( volatile int16* Value, int16 Amount )
	{
		return (int16)::_InterlockedExchangeAdd16((short*)Value, (short)Amount);
	}

	static FORCEINLINE int32 InterlockedAdd( volatile int32* Value, int32 Amount )
	{
		return (int32)::_InterlockedExchangeAdd((long*)Value, (long)Amount);
	}

	static FORCEINLINE int64 InterlockedAdd( volatile int64* Value, int64 Amount )
	{
		#if PLATFORM_64BITS
			return (int64)::_InterlockedExchangeAdd64((int64*)Value, (int64)Amount);
		#else
			// No explicit instruction for 64-bit atomic add on 32-bit processors; has to be implemented in terms of CMPXCHG8B
			for (;;)
			{
				int64 OldValue = *Value;
				if (_InterlockedCompareExchange64(Value, OldValue + Amount, OldValue) == OldValue)
				{
					return OldValue;
				}
			}
		#endif
	}

	static FORCEINLINE int8 InterlockedExchange( volatile int8* Value, int8 Exchange )
	{
		return (int8)::_InterlockedExchange8((char*)Value, (char)Exchange);
	}

	static FORCEINLINE int16 InterlockedExchange( volatile int16* Value, int16 Exchange )
	{
		return (int16)::_InterlockedExchange16((short*)Value, (short)Exchange);
	}

	static FORCEINLINE int32 InterlockedExchange( volatile int32* Value, int32 Exchange )
	{
		return (int32)::_InterlockedExchange((long*)Value, (long)Exchange);
	}

	static FORCEINLINE int64 InterlockedExchange( volatile int64* Value, int64 Exchange )
	{
		#if PLATFORM_64BITS
			return (int64)::_InterlockedExchange64((long long*)Value, (long long)Exchange);
		#else
			// No explicit instruction for 64-bit atomic exchange on 32-bit processors; has to be implemented in terms of CMPXCHG8B
			for (;;)
			{
				int64 OldValue = *Value;
				if (_InterlockedCompareExchange64(Value, Exchange, OldValue) == OldValue)
				{
					return OldValue;
				}
			}
		#endif
	}

	static FORCEINLINE int8 InterlockedCompareExchange( volatile int8* Dest, int8 Exchange, int8 Comparand )
	{
		return (int8)::_InterlockedCompareExchange8((char*)Dest, (char)Exchange, (char)Comparand);
	}

	static FORCEINLINE int16 InterlockedCompareExchange( volatile int16* Dest, int16 Exchange, int16 Comparand )
	{
		return (int16)::_InterlockedCompareExchange16((short*)Dest, (short)Exchange, (short)Comparand);
	}

	static FORCEINLINE int32 InterlockedCompareExchange( volatile int32* Dest, int32 Exchange, int32 Comparand )
	{
		return (int32)::_InterlockedCompareExchange((long*)Dest, (long)Exchange, (long)Comparand);
	}
};

typedef FWindowsPlatformAtomics FPlatformAtomics;
