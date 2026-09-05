// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/HAL/PlatformMemory.h"
#include "Engine/Source/Runtime/Core/Public/HAL/MemoryBase.h"
#include "Engine/Source/Runtime/Core/Public/Templates/IsPointer.h"

#include <cstdlib>

struct CORE_API FMemory
{
	/** @name Memory functions (wrapper for FPlatformMemory) */

	static FORCEINLINE void* Memmove( void* Dest, const void* Src, SIZE_T Count )
	{
		return FPlatformMemory::Memmove( Dest, Src, Count );
	}

	static FORCEINLINE int32 Memcmp( const void* Buf1, const void* Buf2, SIZE_T Count )
	{
		return FPlatformMemory::Memcmp( Buf1, Buf2, Count );
	}

	static FORCEINLINE void* Memset(void* Dest, uint8 Char, SIZE_T Count)
	{
		return FPlatformMemory::Memset( Dest, Char, Count );
	}

	template< class T >
	static FORCEINLINE void Memset( T& Src, uint8 ValueToSet )
	{
		static_assert( !TIsPointer<T>::Value, "For pointers use the three parameters function");
		Memset( &Src, ValueToSet, sizeof( T ) );
	}

	static FORCEINLINE void* Memzero(void* Dest, SIZE_T Count)
	{
		return FPlatformMemory::Memzero( Dest, Count );
	}

	template< class T >
	static FORCEINLINE void Memzero( T& Src )
	{
		static_assert( !TIsPointer<T>::Value, "For pointers use the two parameters function");
		Memzero( &Src, sizeof( T ) );
	}

	static FORCEINLINE void* Memcpy(void* Dest, const void* Src, SIZE_T Count)
	{
		return FPlatformMemory::Memcpy(Dest,Src,Count);
	}

	template< class T >
	static FORCEINLINE void Memcpy( T& Dest, const T& Src )
	{
		static_assert( !TIsPointer<T>::Value, "For pointers use the three parameters function");
		Memcpy( &Dest, &Src, sizeof( T ) );
	}

	static FORCEINLINE void* BigBlockMemcpy(void* Dest, const void* Src, SIZE_T Count)
	{
		return FPlatformMemory::BigBlockMemcpy(Dest,Src,Count);
	}

	static FORCEINLINE void* StreamingMemcpy(void* Dest, const void* Src, SIZE_T Count)
	{
		return FPlatformMemory::StreamingMemcpy(Dest,Src,Count);
	}

	//
	// C style memory allocation stubs that fall back to C runtime
	//
	static FORCEINLINE void* SystemMalloc(SIZE_T Size)
	{
		return ::malloc(Size);
	}

	static FORCEINLINE void SystemFree(void* Ptr)
	{
		::free(Ptr);
	}

	//
	// C style memory allocation stubs.
	//

	static FORCEINLINE void* Malloc(SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		constexpr uintptr_t Offset = 0xC9B198;
		return reinterpret_cast<void* (*)(SIZE_T, uint32)>(ImageBase + Offset)(Count, Alignment);
	}

	static FORCEINLINE void* Realloc(void* Original, SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		constexpr uintptr_t Offset = 0xC2BAC8;
		return reinterpret_cast<void* (*)(void*, SIZE_T, uint32)>(ImageBase + Offset)(Original, Count, Alignment);
	}

	static FORCEINLINE void Free(void* Original)
	{
		constexpr uintptr_t Offset = 0xC3DF60;
		reinterpret_cast<void (*)(void*)>(ImageBase + Offset)(Original);
	}

	static FORCEINLINE SIZE_T QuantizeSize(SIZE_T Count, uint32 Alignment = DEFAULT_ALIGNMENT)
	{
		constexpr uintptr_t Offset = 0xCB0110;
		return reinterpret_cast<SIZE_T (*)(SIZE_T, uint32)>(ImageBase + Offset)(Count, Alignment);
	}
};
