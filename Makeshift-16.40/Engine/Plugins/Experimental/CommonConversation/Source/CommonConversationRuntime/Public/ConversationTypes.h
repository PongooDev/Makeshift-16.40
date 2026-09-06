// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

//////////////////////////////////////////////////////////////////////

struct FConversationBranchPointBuilder : public FNoncopyable
{
	void AddChoice(const FConversationContext& InContext, FClientConversationOptionEntry&& InChoice);

	int32 Num() const { return BranchPoints.Num(); }
	const TArray<FConversationBranchPoint>& GetBranches() const { return BranchPoints; }

private:
	TArray<FConversationBranchPoint> BranchPoints;
};
static_assert(sizeof(FConversationBranchPointBuilder) == 0x000010, "Wrong size on FConversationBranchPointBuilder");
