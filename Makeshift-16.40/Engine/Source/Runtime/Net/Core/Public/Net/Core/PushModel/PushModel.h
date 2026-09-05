// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Net/Core/Public/Net/Core/PushModel/PushModelMacros.h"

#if WITH_PUSH_MODEL

#else // WITH_PUSH_MODEL

#define MARK_PROPERTY_DIRTY(Object, Property)
#define MARK_PROPERTY_DIRTY_STATIC_ARRAY_INDEX(Object, RepIndex, ArrayIndex)
#define MARK_PROPERTY_DIRTY_STATIC_ARRAY(Object, RepIndex, ArrayIndex)

#define MARK_PROPERTY_DIRTY_FROM_NAME(ClassName, PropertyName, Object)
#define MARK_PROPERTY_DIRTY_FROM_NAME_STATIC_ARRAY_INDEX(ClassName, PropertyName, ArrayIndex, Object)
#define MARK_PROPERTY_DIRTY_FROM_NAME_STATIC_ARRAY(ClassName, PropertyName, ArrayIndex, Object)


#define GET_PROPERTY_REP_INDEX(ClassName, PropertyName) INDEX_NONE
#define GET_PROPERTY_REP_INDEX_STATIC_ARRAY(ClassName, PropertyName, ArrayIndex) INDEX_NONE

#define IS_PUSH_MODEL_ENABLED() false
#define PUSH_MAKE_BP_PROPERTIES_PUSH_MODEL() false

#endif
