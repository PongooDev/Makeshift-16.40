// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Math/UnrealMathUtility.h"

// Platform specific vector intrinsics include.
#if WITH_DIRECTXMATH
#define SIMD_ALIGNMENT (16)
#include "Math/UnrealMathDirectX.h"
#elif PLATFORM_ENABLE_VECTORINTRINSICS
#define SIMD_ALIGNMENT (16)
#include "Engine/Source/Runtime/Core/Public/Math/UnrealMathSSE.h"
#elif PLATFORM_ENABLE_VECTORINTRINSICS_NEON
#define SIMD_ALIGNMENT (16)
#include "Math/UnrealMathNeon.h"
#else
#define SIMD_ALIGNMENT (4)
#include "Math/UnrealMathFPU.h"
#endif
