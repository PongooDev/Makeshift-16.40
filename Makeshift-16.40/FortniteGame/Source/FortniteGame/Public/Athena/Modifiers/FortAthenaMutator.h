#include "pch.h"

struct FMutatorContextIterator
{
	FMutatorContext MutatorContext;
	int32 CurrentActorIndex;
	int32 CurrentMutatorIndex;
	TArray<AFortGameplayMutator*> CurrentMutators;
	AFortAthenaMutator* CurrentMutator;
	bool bOnlyActive;

	FMutatorContextIterator(const AActor* MutatorTarget, const AActor* MutatorOwner, bool bInOnlyActive)
	{
		void (*Fn)(FMutatorContextIterator*, const AActor*, const AActor*, bool) = decltype(Fn)(ImageBase + 0xC90D40);
		Fn(this, MutatorTarget, MutatorOwner, bInOnlyActive);
	}

	~FMutatorContextIterator()
	{
		if (CurrentMutator)
		{
			CurrentMutator->ApplyContextActor(nullptr);
			CurrentMutator = nullptr;
		}
	}

	FMutatorContextIterator(const FMutatorContextIterator&) = delete;
	FMutatorContextIterator& operator=(const FMutatorContextIterator&) = delete;

	void SetCurrentMutator(int32 InMutatorIndex)
	{
		void (*Fn)(FMutatorContextIterator*, int32) = decltype(Fn)(ImageBase + 0x45AAAC4);
		Fn(this, InMutatorIndex);
	}
};
static_assert(offsetof(FMutatorContextIterator, CurrentMutatorIndex) == 0x00001C, "Member 'FMutatorContextIterator::CurrentMutatorIndex' has a wrong offset!");
static_assert(offsetof(FMutatorContextIterator, CurrentMutator) == 0x000030, "Member 'FMutatorContextIterator::CurrentMutator' has a wrong offset!");
static_assert(sizeof(FMutatorContextIterator) == 0x000040, "Wrong size on FMutatorContextIterator");
