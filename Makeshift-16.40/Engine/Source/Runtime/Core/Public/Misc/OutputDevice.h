// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogVerbosity.h"
#include "Engine/Source/Runtime/Core/Public/Misc/CoreMiscDefines.h"
#include "Engine/Source/Runtime/Core/Public/Misc/VarArgs.h"
#include "Engine/Source/Runtime/Core/Public/Templates/IsValidVariadicFunctionArg.h"
#include "Engine/Source/Runtime/Core/Public/Templates/AndOrNot.h"
#include "Engine/Source/Runtime/Core/Public/Templates/IsArrayOrRefOfType.h"

class FArchive;

// An output device.
class CORE_API FOutputDevice
{
public:
	FOutputDevice()
		 : bSuppressEventTag      (false)
		 , bAutoEmitLineTerminator(true)
	{}

	FOutputDevice(FOutputDevice&&) = default;
	FOutputDevice(const FOutputDevice&) = default;
	FOutputDevice& operator=(FOutputDevice&&) = default;
	FOutputDevice& operator=(const FOutputDevice&) = default;

	virtual ~FOutputDevice() = default;

	// FOutputDevice interface.
	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category ) = 0;
	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category, const double Time )
	{
		Serialize( V, Verbosity, Category );
	}

	virtual void Flush()
	{
	}

	/**
	 * Closes output device and cleans up. This can't happen in the destructor
	 * as we might have to call "delete" which cannot be done for static/ global
	 * objects.
	 */
	virtual void TearDown()
	{
	}

	void SetSuppressEventTag(bool bInSuppressEventTag)
	{
		bSuppressEventTag = bInSuppressEventTag;
	}
	FORCEINLINE bool GetSuppressEventTag() const	{	return bSuppressEventTag;	}
	void SetAutoEmitLineTerminator(bool bInAutoEmitLineTerminator)
	{
		bAutoEmitLineTerminator = bInAutoEmitLineTerminator;
	}
	FORCEINLINE bool GetAutoEmitLineTerminator() const	{	return bAutoEmitLineTerminator;	}

	/**
	 * Dumps the contents of this output device's buffer to an archive (supported by output device that have a memory buffer)
	 * @param Ar Archive to dump the buffer to
	 */
	virtual void Dump(class FArchive& Ar)
	{
	}

	/**
	* @return whether this output device is a memory-only device
	*/
	virtual bool IsMemoryOnly() const
	{
		return false;
	}

	/**
	 * @return whether this output device can be used on any thread.
	 */
	virtual bool CanBeUsedOnAnyThread() const
	{
		return false;
	}

	/**
	* @return whether this output device can be used from multiple threads simultaneously without any locking
	*/
	virtual bool CanBeUsedOnMultipleThreads() const
	{
		return false;
	}

	// Simple text printing.
	void Log( const TCHAR* S );
	void Log( ELogVerbosity::Type Verbosity, const TCHAR* S );
	void Log( const FName& Category, ELogVerbosity::Type Verbosity, const TCHAR* Str );
	void Log( const FString& S );
	void Log( ELogVerbosity::Type Verbosity, const FString& S );
	void Log( const FName& Category, ELogVerbosity::Type Verbosity, const FString& S );

private:
	void VARARGS LogfImpl(const TCHAR* Fmt, ...);
	void VARARGS LogfImpl(ELogVerbosity::Type Verbosity, const TCHAR* Fmt, ...);
	void VARARGS CategorizedLogfImpl(const FName& Category, ELogVerbosity::Type Verbosity, const TCHAR* Fmt, ...);

public:
	template <typename FmtType>
	void Logf(const FmtType& Fmt)
	{
		static_assert(TIsArrayOrRefOfType<FmtType, TCHAR>::Value, "Formatting string must be a TCHAR array.");
		return Log(Fmt);
	}

	template <typename FmtType, typename... Types>
	FORCEINLINE void Logf(const FmtType& Fmt, Types... Args)
	{
		static_assert(TIsArrayOrRefOfType<FmtType, TCHAR>::Value, "Formatting string must be a TCHAR array.");
		static_assert(TAnd<TIsValidVariadicFunctionArg<Types>...>::Value, "Invalid argument(s) passed to FOutputDevice::Logf");

		LogfImpl(Fmt, Args...);
	}

	template <typename FmtType, typename... Types>
	FORCEINLINE void Logf(ELogVerbosity::Type Verbosity, const FmtType& Fmt, Types... Args)
	{
		static_assert(TIsArrayOrRefOfType<FmtType, TCHAR>::Value, "Formatting string must be a TCHAR array.");
		static_assert(TAnd<TIsValidVariadicFunctionArg<Types>...>::Value, "Invalid argument(s) passed to FOutputDevice::Logf");

		LogfImpl(Verbosity, Fmt, Args...);
	}

	template <typename FmtType, typename... Types>
	FORCEINLINE void CategorizedLogf(const FName& Category, ELogVerbosity::Type Verbosity, const FmtType& Fmt, Types... Args)
	{
		static_assert(TIsArrayOrRefOfType<FmtType, TCHAR>::Value, "Formatting string must be a TCHAR array.");
		static_assert(TAnd<TIsValidVariadicFunctionArg<Types>...>::Value, "Invalid argument(s) passed to FOutputDevice::CategorizedLogf");

		CategorizedLogfImpl(Category, Verbosity, Fmt, Args...);
	}

protected:
	/** Whether to output the 'Log: ' type front... */
	bool bSuppressEventTag;
	/** Whether to output a line-terminator after each log call... */
	bool bAutoEmitLineTerminator;
};
