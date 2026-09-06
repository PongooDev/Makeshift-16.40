// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	Stack.h: Kismet VM execution stack definition.
=============================================================================*/

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Script.h"
#include "Engine/Source/Runtime/Core/Public/Misc/OutputDevice.h"

/**
 * Property data type enums.
 * @warning: if values in this enum are modified, you must update:
 * - FPropertyBase::GetSize() hardcodes the sizes for each property type
 */
enum EPropertyType
{
	CPT_None,
	CPT_Byte,
	CPT_UInt16,
	CPT_UInt32,
	CPT_UInt64,
	CPT_Int8,
	CPT_Int16,
	CPT_Int,
	CPT_Int64,
	CPT_Bool,
	CPT_Bool8,
	CPT_Bool16,
	CPT_Bool32,
	CPT_Bool64,
	CPT_Float,
	CPT_ObjectReference,
	CPT_Name,
	CPT_Delegate,
	CPT_Interface,
	CPT_Unused_Index_19,
	CPT_Struct,
	CPT_Unused_Index_21,
	CPT_Unused_Index_22,
	CPT_String,
	CPT_Text,
	CPT_MulticastDelegate,
	CPT_WeakObjectReference,
	CPT_LazyObjectReference,
	CPT_SoftObjectReference,
	CPT_Double,
	CPT_Map,
	CPT_Set,
	CPT_FieldPath,

	CPT_MAX
};



/*-----------------------------------------------------------------------------
	Execution stack helpers.
-----------------------------------------------------------------------------*/

//
// Information remembered about an Out parameter.
//
struct FOutParmRec
{
	FProperty* Property;
	uint8*      PropAddr;
	FOutParmRec* NextOutParm;
};

//
// Information about script execution at one stack level.
//

struct FFrame : public FOutputDevice
{	
public:
	// Variables.
	UFunction* Node;
	UObject* Object;
	uint8* Code;
	uint8* Locals;

	FProperty* MostRecentProperty;
	uint8* MostRecentPropertyAddress;

	/** The execution flow stack for compiled Kismet code */
	FlowStackType FlowStack;

	/** Previous frame on the stack */
	FFrame* PreviousFrame;

	/** contains information on any out parameters */
	FOutParmRec* OutParms;

	/** If a class is compiled in then this is set to the property chain for compiled-in functions. In that case, we follow the links to setup the args instead of executing by code. */
	FField* PropertyChainForCompiledIn;

	/** Currently executed native function */
	UFunction* CurrentNativeFunction;

	bool bArrayContextFailed;
public:

	// Functions.
	COREUOBJECT_API void Step( UObject* Context, RESULT_DECL );

	/** Replacement for Step that uses an explicitly specified property to unpack arguments **/
	COREUOBJECT_API void StepExplicitProperty(void*const Result, FProperty* Property);

	/** Replacement for Step that checks the for byte code, and if none exists, then PropertyChainForCompiledIn is used. Also, makes an effort to verify that the params are in the correct order and the types are compatible. **/
	template<class TProperty>
	FORCEINLINE_DEBUGGABLE void StepCompiledIn(void* Result);
	FORCEINLINE_DEBUGGABLE void StepCompiledIn(void* Result, const FFieldClass* ExpectedPropertyType);

	/** Replacement for Step that checks the for byte code, and if none exists, then PropertyChainForCompiledIn is used. Also, makes an effort to verify that the params are in the correct order and the types are compatible. **/
	template<class TProperty, typename TNativeType>
	FORCEINLINE_DEBUGGABLE TNativeType& StepCompiledInRef(void*const TemporaryBuffer);

	/** Returns the current script op code */
	const uint8 PeekCode() const { return *Code; }

	/** Skips over the number of op codes specified by NumOps */
	void SkipCode(const int32 NumOps) { Code += NumOps; }

};


/*-----------------------------------------------------------------------------
	FFrame implementation.
-----------------------------------------------------------------------------*/

/**
 * Replacement for Step that checks the for byte code, and if none exists, then PropertyChainForCompiledIn is used.
 * Also makes an effort to verify that the params are in the correct order and the types are compatible.
 **/
template<class TProperty>
FORCEINLINE_DEBUGGABLE void FFrame::StepCompiledIn(void* Result)
{
	StepCompiledIn(Result, TProperty::StaticClass());
}

FORCEINLINE_DEBUGGABLE void FFrame::StepCompiledIn(void* Result, const FFieldClass* ExpectedPropertyType)
{
	if (Code)
	{
		Step(Object, Result);
	}
	else
	{
		checkSlow(ExpectedPropertyType && ExpectedPropertyType->IsChildOf(FProperty::StaticClass()));
		checkSlow(PropertyChainForCompiledIn && PropertyChainForCompiledIn->IsA(ExpectedPropertyType));
		FProperty* Property = (FProperty*)PropertyChainForCompiledIn;
		PropertyChainForCompiledIn = Property->Next;
		StepExplicitProperty(Result, Property);
	}
}

template<class TProperty, typename TNativeType>
FORCEINLINE_DEBUGGABLE TNativeType& FFrame::StepCompiledInRef(void*const TemporaryBuffer)
{
	MostRecentPropertyAddress = NULL;

	if (Code)
	{
		Step(Object, TemporaryBuffer);
	}
	else
	{
		checkSlow(CastField<TProperty>(PropertyChainForCompiledIn) && CastField<FProperty>(PropertyChainForCompiledIn));
		TProperty* Property = (TProperty*)PropertyChainForCompiledIn;
		PropertyChainForCompiledIn = Property->Next;
		StepExplicitProperty(TemporaryBuffer, Property);
	}

	return (MostRecentPropertyAddress != NULL) ? *(TNativeType*)(MostRecentPropertyAddress) : *(TNativeType*)TemporaryBuffer;
}
