// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/*--------------------------------------------------------------------------------
	Build configuration coming from UBT, do not modify
--------------------------------------------------------------------------------*/

// Set any configuration not defined by UBT to zero
#ifndef UE_BUILD_DEBUG
	#define UE_BUILD_DEBUG				0
#endif
#ifndef UE_BUILD_DEVELOPMENT
	#define UE_BUILD_DEVELOPMENT		1
#endif
#ifndef UE_BUILD_TEST
	#define UE_BUILD_TEST				0
#endif
#ifndef UE_BUILD_SHIPPING
	#define UE_BUILD_SHIPPING			0
#endif
#ifndef UE_GAME
	#define UE_GAME						1
#endif
#ifndef UE_EDITOR
	#define UE_EDITOR					0
#endif
#ifndef UE_BUILD_SHIPPING_WITH_EDITOR
	#define UE_BUILD_SHIPPING_WITH_EDITOR 0
#endif
#ifndef UE_BUILD_DOCS
	#define UE_BUILD_DOCS				0
#endif

/**
 *   Whether compiling for dedicated server or not.
 */
#ifndef UE_SERVER
	#define UE_SERVER					0
#endif

// Ensure that we have one, and only one build config coming from UBT
#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING != 1
	#error Exactly one of [UE_BUILD_DEBUG UE_BUILD_DEVELOPMENT UE_BUILD_TEST UE_BUILD_SHIPPING] should be defined to be 1
#endif

/*----------------------------------------------------------------------------
	Mandatory bridge options coming from UBT, do not modify directly!
----------------------------------------------------------------------------*/

/**
 * Whether we are compiling with the editor; must be defined by UBT
 */
#ifndef WITH_EDITOR
	#define WITH_EDITOR	0 // for auto-complete
#endif
/**
 * Whether we are compiling with the engine; must be defined by UBT
 */
#ifndef WITH_ENGINE
	#define WITH_ENGINE	1
#endif
/**
 *	Whether we are compiling with developer tools; must be defined by UBT
 */
#ifndef WITH_UNREAL_DEVELOPER_TOOLS
	#define WITH_UNREAL_DEVELOPER_TOOLS		0	// for auto-complete
#endif
/**
 *	Whether we are compiling with plugin support; must be defined by UBT
 */
#ifndef WITH_PLUGIN_SUPPORT
	#define WITH_PLUGIN_SUPPORT		0	// for auto-complete
#endif
/**
* Whether we want a monolithic build (no DLLs); must be defined by UBT
*/
#ifndef IS_MONOLITHIC
	#define IS_MONOLITHIC 1
#endif
/**
* Whether we want a program (shadercompilerworker, fileserver) or a game; must be defined by UBT
*/
#ifndef IS_PROGRAM
	#define IS_PROGRAM 0 // for autocomplete
#endif
#ifndef USING_CODE_ANALYSIS
	#define USING_CODE_ANALYSIS 0
#endif
#ifndef PLATFORM_CPU_ARM_FAMILY
	#define PLATFORM_CPU_ARM_FAMILY 0
#endif

/*--------------------------------------------------------------------------------
	Basic options that by default depend on the build configuration and platform

	DO_GUARD_SLOW									If true, then checkSlow, checkfSlow and verifySlow are compiled into the executable.
	DO_ENSURE										If true, then ensure, ensureAlways, ensureMsgf and ensureAlwaysMsgf are compiled into the executables
	ALLOW_DEBUG_FILES								If true, then debug files like screen shots and profiles can be saved from the executable.
	NO_LOGGING										If true, then no logs or text output will be produced
--------------------------------------------------------------------------------*/

#if UE_BUILD_DEBUG
	#ifndef DO_GUARD_SLOW
		#define DO_GUARD_SLOW									1
	#endif
	#ifndef DO_CHECK
		#define DO_CHECK										1
	#endif
	#ifndef DO_ENSURE
		#define DO_ENSURE										1
	#endif
	#ifndef STATS
		#define STATS											0
	#endif
	#ifndef ALLOW_DEBUG_FILES
		#define ALLOW_DEBUG_FILES								1
	#endif
	#ifndef ALLOW_CONSOLE
		#define ALLOW_CONSOLE									1
	#endif
	#ifndef NO_LOGGING
		#define NO_LOGGING										0
	#endif
#elif UE_BUILD_DEVELOPMENT
	#ifndef DO_GUARD_SLOW
		#define DO_GUARD_SLOW									0
	#endif
	#ifndef DO_CHECK
		#define DO_CHECK										1
	#endif
	#ifndef DO_ENSURE
		#define DO_ENSURE										1
	#endif
	#ifndef STATS
		#define STATS											0
	#endif
	#ifndef ALLOW_DEBUG_FILES
		#define ALLOW_DEBUG_FILES								1
	#endif
	#ifndef ALLOW_CONSOLE
		#define ALLOW_CONSOLE									1
	#endif
	#ifndef NO_LOGGING
		#define NO_LOGGING										0
	#endif
#elif UE_BUILD_TEST
	#ifndef DO_GUARD_SLOW
		#define DO_GUARD_SLOW									0
	#endif
	#ifndef DO_CHECK
		#define DO_CHECK										USE_CHECKS_IN_SHIPPING
	#endif
	#ifndef DO_ENSURE
		#define DO_ENSURE										USE_ENSURES_IN_SHIPPING
	#endif
	#ifndef STATS
		#define STATS											0
	#endif
	#ifndef ALLOW_DEBUG_FILES
		#define ALLOW_DEBUG_FILES								1
	#endif
	#ifndef ALLOW_CONSOLE
		#define ALLOW_CONSOLE									1
	#endif
	#ifndef NO_LOGGING
		#define NO_LOGGING										!USE_LOGGING_IN_SHIPPING
	#endif
#elif UE_BUILD_SHIPPING
	#ifndef DO_GUARD_SLOW
		#define DO_GUARD_SLOW									0
	#endif
	#ifndef DO_CHECK
		#define DO_CHECK										USE_CHECKS_IN_SHIPPING
	#endif
	#ifndef DO_ENSURE
		#define DO_ENSURE										USE_ENSURES_IN_SHIPPING
	#endif
	#ifndef STATS
		#define STATS											0
	#endif
	#ifndef ALLOW_DEBUG_FILES
		#define ALLOW_DEBUG_FILES								0
	#endif
	#ifndef ALLOW_CONSOLE
		#define ALLOW_CONSOLE									0
	#endif
	#ifndef NO_LOGGING
		#define NO_LOGGING										!USE_LOGGING_IN_SHIPPING
	#endif
#else
	#error Exactly one of [UE_BUILD_DEBUG UE_BUILD_DEVELOPMENT UE_BUILD_TEST UE_BUILD_SHIPPING] should be defined to be 1
#endif

#ifndef USE_CHECKS_IN_SHIPPING
	#define USE_CHECKS_IN_SHIPPING 0
#endif
/** If not defined follow the CHECK behavior since previously ensures were compiled in with checks */
#ifndef USE_ENSURES_IN_SHIPPING
	#define USE_ENSURES_IN_SHIPPING USE_CHECKS_IN_SHIPPING
#endif
/** If not specified, disable logging in shipping */
#ifndef USE_LOGGING_IN_SHIPPING
	#define USE_LOGGING_IN_SHIPPING 0
#endif

/** Whether to use the null RHI. */
#ifndef USE_NULL_RHI
	#define USE_NULL_RHI 0
#endif

#ifndef UE_TRACE_ENABLED
	#define UE_TRACE_ENABLED 0
#endif

/**
 *	Optionally enable support for named events from the stat macros without the stat system overhead
 *	This will attempt to disable regular stats system and use named events instead
 */
#ifndef ENABLE_STATNAMEDEVENTS
	#define ENABLE_STATNAMEDEVENTS	0
#endif

#ifndef ENABLE_STATNAMEDEVENTS_UOBJECT
	#define ENABLE_STATNAMEDEVENTS_UOBJECT 0
#endif
