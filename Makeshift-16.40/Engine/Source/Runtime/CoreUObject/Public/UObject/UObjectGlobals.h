// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/ObjectMacros.h"

struct FStaticConstructObjectParameters;
class FObjectInstancingGraph;

/** Returns the transient top-level package, which is useful for temporarily storing objects that should never be saved */
inline class UPackage* GetTransientPackage()
{
	return *reinterpret_cast<class UPackage**>(ImageBase + 0x9478048);
}

struct FStaticConstructObjectParameters
{
	/** The class of the object to create */
	const UClass* Class;

	/** The object to create this object within (the Outer property for the new object will be set to the value specified here). */
	UObject* Outer;

	/** The name to give the new object.If no value(NAME_None) is specified, the object will be given a unique name in the form of ClassName_#. */
	FName Name;

	/** The ObjectFlags to assign to the new object. some flags can affect the behavior of constructing the object. */
	::EObjectFlags SetFlags = ::RF_NoFlags;

	/** The InternalObjectFlags to assign to the new object. some flags can affect the behavior of constructing the object. */
	::EInternalObjectFlags InternalSetFlags = ::EInternalObjectFlags::None;

	/** If true, copy transient from the class defaults instead of the pass in archetype ptr(often these are the same) */
	bool bCopyTransientsFromClassDefaults = false;

	/** If true, Template is guaranteed to be an archetype */
	bool bAssumeTemplateIsArchetype = false;

	/**
	 * If specified, the property values from this object will be copied to the new object, and the new object's ObjectArchetype value will be set to this object.
	 * If nullptr, the class default object is used instead.
	 */
	UObject* Template = nullptr;

	/** Contains the mappings of instanced objects and components to their templates */
	FObjectInstancingGraph* InstanceGraph = nullptr;

	/** Assign an external Package to the created object if non-null */
	UPackage* ExternalPackage = nullptr;

	FStaticConstructObjectParameters(const UClass* InClass)
		: Class(InClass)
		, Outer((UObject*)GetTransientPackage())
	{
	}
};
static_assert(sizeof(FStaticConstructObjectParameters) == 0x40, "Wrong size on FStaticConstructObjectParameters");

/**
 * Create a new instance of an object.  The returned object will be fully initialized.  If InFlags contains RF_NeedsLoad (indicating that the object still needs to load its object data from disk), components
 * are not instanced (this will instead occur in PostLoad()).  The different between StaticConstructObject and StaticAllocateObject is that StaticConstructObject will also call the class constructor on the object
 * and instance any components.
 *
 * @param	Params		The parameters to use when construction the object. @see FStaticConstructObjectParameters
 *
 * @return	A pointer to a fully initialized object of the specified class.
 */
inline UObject* StaticConstructObject_Internal(const FStaticConstructObjectParameters& Params)
{
	UObject* (*Fn)(const FStaticConstructObjectParameters*) = decltype(Fn)(ImageBase + 0xC40BB4);
	return Fn(&Params);
}

/**
 * Convenience template for constructing a gameplay object
 *
 * @param	Outer		the outer for the new object.  If not specified, object will be created in the transient package.
 * @param	Class		the class of object to construct
 * @param	Name		the name for the new object.  If not specified, the object will be given a transient name via MakeUniqueObjectName
 * @param	Flags		the object flags to apply to the new object
 * @param	Template	the object to use for initializing the new object.  If not specified, the class's default object will be used
 * @param	bCopyTransientsFromClassDefaults	if true, copy transient from the class defaults instead of the pass in archetype ptr (often these are the same)
 * @param	InInstanceGraph						contains the mappings of instanced objects and components to their templates
 * @param	ExternalPackage						Assign an external Package to the created object if non-null
 *
 * @return	a pointer of type T to a new object of the specified class
 */
template< class T >
T* NewObject(UObject* Outer, const UClass* Class, FName Name = FName(), ::EObjectFlags Flags = ::RF_NoFlags, UObject* Template = nullptr, bool bCopyTransientsFromClassDefaults = false, FObjectInstancingGraph* InInstanceGraph = nullptr, UPackage* ExternalPackage = nullptr)
{
	FStaticConstructObjectParameters Params(Class);
	Params.Outer = Outer;
	Params.Name = Name;
	Params.SetFlags = Flags;
	Params.Template = Template;
	Params.bCopyTransientsFromClassDefaults = bCopyTransientsFromClassDefaults;
	Params.InstanceGraph = InInstanceGraph;
	Params.ExternalPackage = ExternalPackage;
	return static_cast<T*>(StaticConstructObject_Internal(Params));
}

template< class T >
T* NewObject(UObject* Outer = (UObject*)GetTransientPackage())
{
	FStaticConstructObjectParameters Params(T::StaticClass());
	Params.Outer = Outer;
	return static_cast<T*>(StaticConstructObject_Internal(Params));
}
