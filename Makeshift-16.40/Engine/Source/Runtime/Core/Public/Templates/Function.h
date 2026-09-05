// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Templates/UnrealTemplate.h"
#include "Engine/Source/Runtime/Core/Public/Templates/Invoke.h"

template <typename FuncType> class TFunctionRef;

namespace UE4Function_Private
{
	/**
	 * A class which is used to instantiate the code needed to call a bound function.
	 */
	template <typename Functor, typename FuncType>
	struct TFunctionRefCaller;

	template <typename Functor, typename Ret, typename... ParamTypes>
	struct TFunctionRefCaller<Functor, Ret (ParamTypes...)>
	{
		static Ret Call(void* Obj, ParamTypes&... Params)
		{
			return Invoke(*(Functor*)Obj, Forward<ParamTypes>(Params)...);
		}
	};

	template <typename Functor, typename... ParamTypes>
	struct TFunctionRefCaller<Functor, void (ParamTypes...)>
	{
		static void Call(void* Obj, ParamTypes&... Params)
		{
			Invoke(*(Functor*)Obj, Forward<ParamTypes>(Params)...);
		}
	};
}

/**
 * TFunctionRef<FuncType>
 *
 * See the class definition for intended usage.
 */
template <typename FuncType>
class TFunctionRef;

template <typename Ret, typename... ParamTypes>
class TFunctionRef<Ret (ParamTypes...)>
{
public:
	/**
	 * Constructor which binds a TFunctionRef to a callable object.
	 */
	template <typename FunctorType>
	TFunctionRef(FunctorType&& InFunc)
	{
		Callable = &UE4Function_Private::TFunctionRefCaller<typename TRemoveReference<FunctorType>::Type, Ret (ParamTypes...)>::Call;
		Ptr      = (void*)&InFunc;
	}

	TFunctionRef(const TFunctionRef&) = default;

	// We delete the assignment operators because we don't want it to be confused with being related to
	// regular C++ reference assignment - i.e. calling the assignment operator of whatever the reference
	// is bound to - because that's not what TFunctionRef does, nor is it even capable of doing that.
	TFunctionRef& operator=(const TFunctionRef&) const = delete;
	~TFunctionRef() = default;

	Ret operator()(ParamTypes... Params) const
	{
		return Callable(Ptr, Params...);
	}

private:
	Ret (*Callable)(void*, ParamTypes&...);
	void* Ptr;
};
