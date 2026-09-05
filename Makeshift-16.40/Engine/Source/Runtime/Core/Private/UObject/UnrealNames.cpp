// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/UObject/NameTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/AssertionMacros.h"

class FNamePool
{
public:
	FNameEntryId Find(EName Ename) const;

private:
	uint8 Pad_0[0x14040];

	// Put constant lookup on separate cache line to avoid it being constantly invalidated by insertion
	alignas(PLATFORM_CACHE_LINE_SIZE) FNameEntryId ENameToEntry[NAME_MaxHardcodedNameIndex] = {};
};

static FNamePool& GetNamePool()
{
	constexpr uintptr_t NamePoolData = 0x9643080;

	return *reinterpret_cast<FNamePool*>(InSDKUtils::GetImageBase() + NamePoolData);
}

FNameEntryId FNameEntryId::FromUnstableInt(uint32 Value)
{
	FNameEntryId Id;
	Id.Value = Value;
	return Id;
}

FNameEntryId FNamePool::Find(EName Ename) const
{
	checkSlow(Ename < NAME_MaxHardcodedNameIndex);
	return ENameToEntry[Ename];
}

FNameEntryId FNameEntryId::FromValidEName(EName Ename)
{
	return GetNamePool().Find(Ename);
}
