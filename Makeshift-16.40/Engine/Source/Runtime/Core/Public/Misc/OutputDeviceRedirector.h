// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/OutputDevice.h"

/*-----------------------------------------------------------------------------
FOutputDeviceRedirector.
-----------------------------------------------------------------------------*/

class FLogAllocator;

/**
* Class used for output redirection to allow logs to show
*/
class CORE_API FOutputDeviceRedirector : public FOutputDevice
{
private:
	template<class T>
	void SerializeImpl(const TCHAR* Data, ELogVerbosity::Type Verbosity, T& CategoryName, double Time);

public:

	/** Initialization constructor. */
	explicit FOutputDeviceRedirector(FLogAllocator* Allocator = nullptr);

	/**
	* Get the GLog singleton
	*/
	static FOutputDeviceRedirector* Get();

	/** Flushes lines buffered by secondary threads. */
	virtual void FlushThreadedLogs();

	/**
	*	Flushes lines buffered by secondary threads.
	*	Only used if a background thread crashed and we needed to push the callstack into the log.
	*/
	virtual void PanicFlushThreadedLogs();

	/**
	* Serializes the current backlog to the specified output device.
	* @param OutputDevice	- Output device that will receive the current backlog
	*/
	virtual void SerializeBacklog(FOutputDevice* OutputDevice);

	/**
	* Enables or disables the backlog.
	* @param bEnable	- Starts saving a backlog if true, disables and discards any backlog if false
	*/
	virtual void EnableBacklog(bool bEnable);

	/**
	* Sets the current thread to be the master thread that prints directly
	* (isn't queued up)
	*/
	virtual void SetCurrentThreadAsMasterThread();

	/**
	* Serializes the passed in data via all current output devices.
	*
	* @param	Data	Text to log
	* @param	Event	Event name used for suppression purposes
	*/
	virtual void Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category, const double Time) override;

	/**
	* Serializes the passed in data via all current output devices.
	*
	* @param	Data	Text to log
	* @param	Event	Event name used for suppression purposes
	*/
	virtual void Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category) override;

	/** Same as Serialize() but FName creation. Only needed to support
	*/
	void RedirectLog(const FName& Category, ELogVerbosity::Type Verbosity, const TCHAR* Data);

	/**
	* Passes on the flush request to all current output devices.
	*/
	void Flush() override;

	/**
	* Closes output device and cleans up. This can't happen in the destructor
	* as we might have to call "delete" which cannot be done for static/ global
	* objects.
	*/
	void TearDown() override;
};
