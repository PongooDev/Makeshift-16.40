// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Templates/EnableIf.h"
#include "Engine/Source/Runtime/Core/Public/Templates/AndOrNot.h"
#include "Engine/Source/Runtime/Core/Public/Templates/IsArithmetic.h"
#include "Engine/Source/Runtime/Core/Public/Templates/IsEnum.h"
#include "Engine/Source/Runtime/Core/Public/Templates/IsPointer.h"
#include "Engine/Source/Runtime/Core/Public/Templates/IsClass.h"
#include "Engine/Source/Runtime/Core/Public/Templates/RemoveReference.h"
#include "Engine/Source/Runtime/Core/Public/Templates/TypeCompatibleBytes.h"
#include "Engine/Source/Runtime/Core/Public/Templates/UnrealTypeTraits.h"
#include "Engine/Source/Runtime/Core/Public/Traits/IsContiguousContainer.h"
#include "Engine/Source/Runtime/Core/Public/HAL/UnrealMemory.h"

/*-----------------------------------------------------------------------------
	Standard templates.
-----------------------------------------------------------------------------*/

/**
 * Chooses between the two parameters based on whether the first is nullptr or not.
 * @return If the first parameter provided is non-nullptr, it is returned; otherwise the second parameter is returned.
 */
template<typename ReferencedType>
FORCEINLINE ReferencedType* IfAThenAElseB(ReferencedType* A,ReferencedType* B)
{
	return A ? A : B;
}

/** branchless pointer selection based on predicate
* return PTRINT(Predicate) ? A : B;
**/
template<typename PredicateType,typename ReferencedType>
FORCEINLINE ReferencedType* IfPThenAElseB(PredicateType Predicate,ReferencedType* A,ReferencedType* B)
{
	return Predicate ? A : B;
}

/** A logical exclusive or function. */
inline bool XOR(bool A, bool B)
{
	return A != B;
}

/**
 * Generically gets the data pointer of a contiguous container
 */
template<typename T, typename = typename TEnableIf<TIsContiguousContainer<T>::Value>::Type>
auto GetData(T&& Container) -> decltype(Container.GetData())
{
	return Container.GetData();
}

template <typename T, SIZE_T N> CONSTEXPR       T* GetData(      T (& Container)[N]) { return Container; }
template <typename T, SIZE_T N> CONSTEXPR       T* GetData(      T (&&Container)[N]) { return Container; }
template <typename T, SIZE_T N> CONSTEXPR const T* GetData(const T (& Container)[N]) { return Container; }
template <typename T, SIZE_T N> CONSTEXPR const T* GetData(const T (&&Container)[N]) { return Container; }

template <typename T>
CONSTEXPR const T* GetData(std::initializer_list<T> List)
{
	return List.begin();
}

/**
* Generically gets the number of items in a contiguous container
*/
template<typename T, typename = typename TEnableIf<TIsContiguousContainer<T>::Value>::Type>
auto GetNum(T&& Container) -> decltype(Container.Num())
{
	return Container.Num();
}

template <typename T, SIZE_T N> CONSTEXPR SIZE_T GetNum(      T (& Container)[N]) { return N; }
template <typename T, SIZE_T N> CONSTEXPR SIZE_T GetNum(      T (&&Container)[N]) { return N; }
template <typename T, SIZE_T N> CONSTEXPR SIZE_T GetNum(const T (& Container)[N]) { return N; }
template <typename T, SIZE_T N> CONSTEXPR SIZE_T GetNum(const T (&&Container)[N]) { return N; }

/**
 * Gets the number of items in an initializer list.
 *
 * The return type is int32 for compatibility with other code in the engine.
 * Realistically, an initializer list should not exceed the limits of int32.
 */
template <typename T>
CONSTEXPR int32 GetNum(std::initializer_list<T> List)
{
	return static_cast<int32>(List.size());
}

template <typename T, uint32 N>
char (&ArrayCountHelper(const T (&)[N]))[N + 1];

// Number of elements in an array.
#define UE_ARRAY_COUNT( array ) (sizeof(ArrayCountHelper(array)) - 1)

// Offset of a struct member.
#ifndef UNREAL_CODE_ANALYZER
// UCA uses clang on Windows. According to C++11 standard, (which in this case clang follows and msvc doesn't)
// forbids using reinterpret_cast in constant expressions. msvc uses reinterpret_cast in offsetof macro,
// while clang uses compiler intrinsic. Calling static_assert(STRUCT_OFFSET(x, y) == SomeValue) causes compiler
// error when using clang on Windows (while including windows headers).
#define STRUCT_OFFSET( struc, member )	offsetof(struc, member)
#else
#define STRUCT_OFFSET( struc, member )	__builtin_offsetof(struc, member)
#endif

/*----------------------------------------------------------------------------
	Standard macros.
----------------------------------------------------------------------------*/

template <typename T> struct TRemovePointer     { typedef T Type; };
template <typename T> struct TRemovePointer<T*> { typedef T Type; };

/**
 * MoveTemp will cast a reference to an rvalue reference.
 * This is UE's equivalent of std::move except that it will not compile when passed an rvalue or
 * const object, because we would prefer to be informed when MoveTemp will have no effect.
 */
template <typename T>
FORCEINLINE typename TRemoveReference<T>::Type&& MoveTemp(T&& Obj)
{
	typedef typename TRemoveReference<T>::Type CastType;

	// Validate that we're not being passed an rvalue or a const object - the former is redundant, the latter is almost certainly a mistake
	static_assert(TIsLValueReferenceType<T>::Value, "MoveTemp called on an rvalue");
	static_assert(!TAreTypesEqual<CastType&, const CastType&>::Value, "MoveTemp called on a const object");

	return (CastType&&)Obj;
}

/**
 * MoveTemp will cast a reference to an rvalue reference.
 * This is UE's equivalent of std::move.  It doesn't static assert like MoveTemp, because it is useful in
 * templates or macros where it's not obvious what the argument is, but you want to take advantage of move semantics
 * where you can but not stop compilation.
 */
template <typename T>
FORCEINLINE typename TRemoveReference<T>::Type&& MoveTempIfPossible(T&& Obj)
{
	typedef typename TRemoveReference<T>::Type CastType;
	return (CastType&&)Obj;
}

/**
 * CopyTemp will enforce the creation of an rvalue which can bind to rvalue reference parameters.
 * Unlike MoveTemp, the source object will never be modifed. (i.e. a copy will be made)
 * There is no std:: equivalent.
 */
template <typename T>
FORCEINLINE T CopyTemp(T& Val)
{
	return const_cast<const T&>(Val);
}

template <typename T>
FORCEINLINE T CopyTemp(const T& Val)
{
	return Val;
}

template <typename T>
FORCEINLINE T&& CopyTemp(T&& Val)
{
	// If we already have an rvalue, just return it unchanged, rather than needlessly creating yet another rvalue from it.
	return MoveTemp(Val);
}

/**
 * Forward will cast a reference to an rvalue reference.
 * This is UE's equivalent of std::forward.
 */
template <typename T>
FORCEINLINE T&& Forward(typename TRemoveReference<T>::Type& Obj)
{
	return (T&&)Obj;
}

template <typename T>
FORCEINLINE T&& Forward(typename TRemoveReference<T>::Type&& Obj)
{
	return (T&&)Obj;
}

/**
 * A traits class which specifies whether a Swap of a given type should swap the bits or use a traditional value-based swap.
 */
template <typename T>
struct TUseBitwiseSwap
{
	// We don't use bitwise swapping for 'register' types because this will force them into memory and be slower.
	enum { Value = !TOrValue<__is_enum(T), TIsPointer<T>, TIsArithmetic<T>>::Value };
};


/**
 * Swap two values.  Assumes the types are trivially relocatable.
 */
template <typename T>
inline typename TEnableIf<TUseBitwiseSwap<T>::Value>::Type Swap(T& A, T& B)
{
	if (LIKELY(&A != &B))
	{
		TTypeCompatibleBytes<T> Temp;
		FMemory::Memcpy(&Temp, &A, sizeof(T));
		FMemory::Memcpy(&A, &B, sizeof(T));
		FMemory::Memcpy(&B, &Temp, sizeof(T));
	}
}

template <typename T>
inline typename TEnableIf<!TUseBitwiseSwap<T>::Value>::Type Swap(T& A, T& B)
{
	T Temp = MoveTemp(A);
	A = MoveTemp(B);
	B = MoveTemp(Temp);
}

template <typename T>
inline void Exchange(T& A, T& B)
{
	Swap(A, B);
}

/**
 * This exists to avoid a Visual Studio bug where using a cast to forward an rvalue reference array argument
 * to a pointer parameter will cause bad code generation.  Wrapping the cast in a function causes the correct
 * code to be generated.
 */
template <typename T, typename ArgType>
FORCEINLINE T StaticCast(ArgType&& Arg)
{
	return static_cast<T>(Arg);
}

/**
 * TRValueToLValueReference converts any rvalue reference type into the equivalent lvalue reference, otherwise returns the same type.
 */
template <typename T> struct TRValueToLValueReference      { typedef T  Type; };
template <typename T> struct TRValueToLValueReference<T&&> { typedef T& Type; };

/**
 * Reverses the order of the bits of a value.
 * This is an TEnableIf'd template to ensure that no undesirable conversions occur.  Overloads for other types can be added in the same way.
 *
 * @param Bits - The value to bit-swap.
 * @return The bit-swapped value.
 */
template <typename T>
FORCEINLINE typename TEnableIf<TAreTypesEqual<T, uint32>::Value, T>::Type ReverseBits( T Bits )
{
	Bits = ( Bits << 16) | ( Bits >> 16);
	Bits = ( (Bits & 0x00ff00ff) << 8 ) | ( (Bits & 0xff00ff00) >> 8 );
	Bits = ( (Bits & 0x0f0f0f0f) << 4 ) | ( (Bits & 0xf0f0f0f0) >> 4 );
	Bits = ( (Bits & 0x33333333) << 2 ) | ( (Bits & 0xcccccccc) >> 2 );
	Bits = ( (Bits & 0x55555555) << 1 ) | ( (Bits & 0xaaaaaaaa) >> 1 );
	return Bits;
}

/** Template for initializing a singleton at the boot. */
template< class T >
struct TForceInitAtBoot
{
	TForceInitAtBoot()
	{
		T::Get();
	}
};

/** Used to avoid cluttering code with ifdefs. */
struct FNoopStruct
{
	FNoopStruct()
	{}

	~FNoopStruct()
	{}
};

/**
 * Removes one level of pointer from a type, e.g.:
 *
 * TRemovePointer<      int32  >::Type == int32
 * TRemovePointer<      int32* >::Type == int32
 * TRemovePointer<      int32**>::Type == int32*
 * TRemovePointer<const int32* >::Type == const int32
 */
