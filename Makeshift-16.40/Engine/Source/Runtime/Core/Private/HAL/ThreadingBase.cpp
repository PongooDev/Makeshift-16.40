// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/HAL/ThreadSingleton.h"
#include "Engine/Source/Runtime/Core/Public/HAL/PlatformAtomics.h"
#include "Engine/Source/Runtime/Core/Public/Misc/AssertionMacros.h"

/*-----------------------------------------------------------------------------
	FThreadSingletonInitializer
-----------------------------------------------------------------------------*/

FTlsAutoCleanup* FThreadSingletonInitializer::Get( TFunctionRef<FTlsAutoCleanup*()> CreateInstance, uint32& TlsSlot )
{
	if (TlsSlot == 0xFFFFFFFF)
	{
		const uint32 ThisTlsSlot = FPlatformTLS::AllocTlsSlot();
		check(FPlatformTLS::IsValidTlsSlot(ThisTlsSlot));
		const uint32 PrevTlsSlot = FPlatformAtomics::InterlockedCompareExchange( (int32*)&TlsSlot, (int32)ThisTlsSlot, 0xFFFFFFFF );
		if (PrevTlsSlot != 0xFFFFFFFF)
		{
			FPlatformTLS::FreeTlsSlot( ThisTlsSlot );
		}
	}
	FTlsAutoCleanup* ThreadSingleton = (FTlsAutoCleanup*)FPlatformTLS::GetTlsValue( TlsSlot );
	if( !ThreadSingleton )
	{
		ThreadSingleton = CreateInstance();
		ThreadSingleton->Register();
		FPlatformTLS::SetTlsValue( TlsSlot, ThreadSingleton );
	}
	return ThreadSingleton;
}

FTlsAutoCleanup* FThreadSingletonInitializer::TryGet(uint32& TlsSlot)
{
	if (TlsSlot == 0xFFFFFFFF)
	{
		return nullptr;
	}

	FTlsAutoCleanup* ThreadSingleton = (FTlsAutoCleanup*)FPlatformTLS::GetTlsValue(TlsSlot);
	return ThreadSingleton;
}

void FTlsAutoCleanup::Register()
{
}

CORE_API bool IsInActualRenderingThread()
{
	constexpr uintptr_t Offset = 0xDCA038;
	return reinterpret_cast<bool (*)()>(ImageBase + Offset)();
}

CORE_API bool IsInParallelRenderingThread()
{
	constexpr uintptr_t Offset = 0x5216690;
	return reinterpret_cast<bool (*)()>(ImageBase + Offset)();
}
