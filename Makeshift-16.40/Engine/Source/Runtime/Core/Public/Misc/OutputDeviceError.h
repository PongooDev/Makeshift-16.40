// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/OutputDevice.h"

// Error device.
class CORE_API FOutputDeviceError : public FOutputDevice
{
public:
	virtual void HandleError()=0;
};
