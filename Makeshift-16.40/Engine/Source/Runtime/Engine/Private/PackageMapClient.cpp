// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/PackageMapClient.h"

bool FNetGUIDCache::SupportsObject( const UObject* Object, const TWeakObjectPtr<UObject>* WeakObjectPtr ) const
{
	constexpr uintptr_t Offset = 0x11CFA10;

	return reinterpret_cast<bool (*)(const FNetGUIDCache*, const UObject*, const TWeakObjectPtr<UObject>*)>(ImageBase + Offset)(this, Object, WeakObjectPtr);
}
