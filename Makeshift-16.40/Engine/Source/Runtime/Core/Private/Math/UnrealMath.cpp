// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Math/VectorRegister.h"

FQuat FRotator::Quaternion() const
{
	//SCOPE_CYCLE_COUNTER(STAT_MathConvertRotatorToQuat);

	DiagnosticCheckNaN();

#if PLATFORM_ENABLE_VECTORINTRINSICS
	const VectorRegister Angles = MakeVectorRegister(Pitch, Yaw, Roll, 0.0f);
	const VectorRegister AnglesNoWinding = VectorMod(Angles, GlobalVectorConstants::Float360);
	const VectorRegister HalfAngles = VectorMultiply(AnglesNoWinding, GlobalVectorConstants::DEG_TO_RAD_HALF);

	VectorRegister SinAngles, CosAngles;
	VectorSinCos(&SinAngles, &CosAngles, &HalfAngles);

	// Vectorized conversion, measured 20% faster than using scalar version after VectorSinCos.
	// Indices within VectorRegister (for shuffles): P=0, Y=1, R=2
	const VectorRegister SR = VectorReplicate(SinAngles, 2);
	const VectorRegister CR = VectorReplicate(CosAngles, 2);

	const VectorRegister SY_SY_CY_CY_Temp = VectorShuffle(SinAngles, CosAngles, 1, 1, 1, 1);

	const VectorRegister SP_SP_CP_CP = VectorShuffle(SinAngles, CosAngles, 0, 0, 0, 0);
	const VectorRegister SY_CY_SY_CY = VectorShuffle(SY_SY_CY_CY_Temp, SY_SY_CY_CY_Temp, 0, 2, 0, 2);

	const VectorRegister CP_CP_SP_SP = VectorShuffle(CosAngles, SinAngles, 0, 0, 0, 0);
	const VectorRegister CY_SY_CY_SY = VectorShuffle(SY_SY_CY_CY_Temp, SY_SY_CY_CY_Temp, 2, 0, 2, 0);

	const uint32 Neg = uint32(1 << 31);
	const uint32 Pos = uint32(0);
	const VectorRegister SignBitsLeft  = MakeVectorRegister(Pos, Neg, Pos, Pos);
	const VectorRegister SignBitsRight = MakeVectorRegister(Neg, Neg, Neg, Pos);
	const VectorRegister LeftTerm  = VectorBitwiseXor(SignBitsLeft , VectorMultiply(CR, VectorMultiply(SP_SP_CP_CP, SY_CY_SY_CY)));
	const VectorRegister RightTerm = VectorBitwiseXor(SignBitsRight, VectorMultiply(SR, VectorMultiply(CP_CP_SP_SP, CY_SY_CY_SY)));

	FQuat RotationQuat;
	const VectorRegister Result = VectorAdd(LeftTerm, RightTerm);
	VectorStoreAligned(Result, &RotationQuat);
#else
	const float DEG_TO_RAD = PI/(180.f);
	const float RADS_DIVIDED_BY_2 = DEG_TO_RAD/2.f;
	float SP, SY, SR;
	float CP, CY, CR;

	const float PitchNoWinding = FMath::Fmod(Pitch, 360.0f);
	const float YawNoWinding = FMath::Fmod(Yaw, 360.0f);
	const float RollNoWinding = FMath::Fmod(Roll, 360.0f);

	FMath::SinCos(&SP, &CP, PitchNoWinding * RADS_DIVIDED_BY_2);
	FMath::SinCos(&SY, &CY, YawNoWinding * RADS_DIVIDED_BY_2);
	FMath::SinCos(&SR, &CR, RollNoWinding * RADS_DIVIDED_BY_2);

	FQuat RotationQuat;
	RotationQuat.X =  CR*SP*SY - SR*CP*CY;
	RotationQuat.Y = -CR*SP*CY - SR*CP*SY;
	RotationQuat.Z =  CR*CP*SY - SR*SP*CY;
	RotationQuat.W =  CR*CP*CY + SR*SP*SY;
#endif // PLATFORM_ENABLE_VECTORINTRINSICS

#if ENABLE_NAN_DIAGNOSTIC || DO_CHECK
	// Very large inputs can cause NaN's. Want to catch this here
	if (RotationQuat.ContainsNaN())
	{
		logOrEnsureNanError(TEXT("Invalid input %s to FRotator::Quaternion - generated NaN output: %s"), *ToString(), *RotationQuat.ToString());
		RotationQuat = FQuat::Identity;
	}
#endif

	return RotationQuat;
}

CORE_API const FRotator FRotator::ZeroRotator(0.f,0.f,0.f);
CORE_API const FQuat FQuat::Identity(0,0,0,1);
