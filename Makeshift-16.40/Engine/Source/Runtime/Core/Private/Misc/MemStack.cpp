// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Misc/MemStack.h"
#include <atomic>

FPageAllocator& FPageAllocator::Get()
{
	static std::atomic<FPageAllocator*> ThePageAllocator;

	FPageAllocator* LocalPageAllocator = ThePageAllocator.load(std::memory_order_acquire);
	if (!LocalPageAllocator)
	{
		static FPageAllocator Instance;
		LocalPageAllocator = &Instance;
		ThePageAllocator.store(LocalPageAllocator, std::memory_order_release);
	}
	return *LocalPageAllocator;
}

void *FPageAllocator::Alloc()
{
	void *Result = FMemory::Malloc(PageSize);
	return Result;
}

void FPageAllocator::Free(void *Mem)
{
	FMemory::Free(Mem);
}

void *FPageAllocator::AllocSmall()
{
	void *Result = FMemory::Malloc(SmallPageSize);
	return Result;
}

void FPageAllocator::FreeSmall(void *Mem)
{
	FMemory::Free(Mem);
}

uint64 FPageAllocator::BytesUsed()
{
	return 0;
}

uint64 FPageAllocator::BytesFree()
{
	return 0;
}

void FPageAllocator::LatchProtectedMode()
{
}

/*-----------------------------------------------------------------------------
	FMemStack implementation.
-----------------------------------------------------------------------------*/

int32 FMemStackBase::GetByteCount() const
{
	int32 Count = 0;
	for( FTaggedMemory* Chunk=TopChunk; Chunk; Chunk=Chunk->Next )
	{
		if( Chunk!=TopChunk )
		{
			Count += Chunk->DataSize;
		}
		else
		{
			Count += UE_PTRDIFF_TO_INT32(Top - Chunk->Data());
		}
	}
	return Count;
}

bool FMemStackBase::ContainsPointer(const void* Pointer) const
{
	const uint8* Ptr = (const uint8*)Pointer;
	for (const FTaggedMemory* Chunk = TopChunk; Chunk; Chunk = Chunk->Next)
	{
		if (Ptr >= Chunk->Data() && Ptr < Chunk->Data() + Chunk->DataSize)
		{
			return true;
		}
	}

	return false;
}

void FMemStackBase::AllocateNewChunk(int32 MinSize)
{
	FTaggedMemory* Chunk=nullptr;
	// Create new chunk.
	int32 TotalSize = MinSize + (int32)sizeof(FTaggedMemory);
	uint32 AllocSize;
	if (TopChunk || TotalSize > FPageAllocator::SmallPageSize)
	{
		AllocSize = AlignArbitrary<int32>(TotalSize, FPageAllocator::PageSize);
		if (AllocSize == FPageAllocator::PageSize)
		{
			Chunk = (FTaggedMemory*)FPageAllocator::Get().Alloc();
		}
		else
		{
			Chunk = (FTaggedMemory*)FMemory::Malloc(AllocSize);
		}
		check(AllocSize != FPageAllocator::SmallPageSize);
	}
	else
	{
		AllocSize = FPageAllocator::SmallPageSize;
		Chunk = (FTaggedMemory*)FPageAllocator::Get().AllocSmall();
	}
	Chunk->DataSize = AllocSize - sizeof(FTaggedMemory);

	Chunk->Next = TopChunk;
	TopChunk    = Chunk;
	Top         = Chunk->Data();
	End         = Top + Chunk->DataSize;
}

void FMemStackBase::FreeChunks(FTaggedMemory* NewTopChunk)
{
	while( TopChunk!=NewTopChunk )
	{
		FTaggedMemory* RemoveChunk = TopChunk;
		TopChunk                   = TopChunk->Next;
		if (RemoveChunk->DataSize + sizeof(FTaggedMemory) == FPageAllocator::PageSize)
		{
			FPageAllocator::Get().Free(RemoveChunk);
		}
		else if (RemoveChunk->DataSize + sizeof(FTaggedMemory) == FPageAllocator::SmallPageSize)
		{
			FPageAllocator::Get().FreeSmall(RemoveChunk);
		}
		else
		{
			FMemory::Free(RemoveChunk);
		}
	}
	Top = nullptr;
	End = nullptr;
	if( TopChunk )
	{
		Top = TopChunk->Data();
		End = Top + TopChunk->DataSize;
	}
}
