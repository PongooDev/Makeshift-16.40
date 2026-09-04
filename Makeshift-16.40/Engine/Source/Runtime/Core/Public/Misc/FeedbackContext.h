// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/OutputDevice.h"

/**
 * A context for displaying modal warning messages.
 */
class CORE_API FFeedbackContext
	: public FOutputDevice
{
};
