// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Turns an preprocessor token into a real string (see UBT_COMPILED_PLATFORM)
#define PREPROCESSOR_TO_STRING(x) PREPROCESSOR_TO_STRING_INNER(x)
#define PREPROCESSOR_TO_STRING_INNER(x) #x

// Concatenates two preprocessor tokens, performing macro expansion on them first
#define PREPROCESSOR_JOIN(x, y) PREPROCESSOR_JOIN_INNER(x, y)
#define PREPROCESSOR_JOIN_INNER(x, y) x##y

// Concatenates the first two preprocessor tokens of a variadic list, after performing macro expansion on them
#define PREPROCESSOR_JOIN_FIRST(x, ...) PREPROCESSOR_JOIN_FIRST_INNER(x, __VA_ARGS__)
#define PREPROCESSOR_JOIN_FIRST_INNER(x, ...) x##__VA_ARGS__

// Expands to the second argument or the third argument if the first argument is 1 or 0 respectively
#define PREPROCESSOR_IF(cond, x, y) PREPROCESSOR_JOIN(PREPROCESSOR_IF_INNER_, cond)(x, y)
#define PREPROCESSOR_IF_INNER_1(x, y) x
#define PREPROCESSOR_IF_INNER_0(x, y) y

// Expands to the parameter list of the macro - used for when you need to pass a comma-separated identifier to another macro as a single parameter
#define PREPROCESSOR_COMMA_SEPARATED(first, second, ...) first, second, ##__VA_ARGS__

// Expands to nothing - used as a placeholder
#define PREPROCESSOR_NOTHING

// Removes a single layer of parentheses from a macro argument if they are present - used to allow
// brackets to be optionally added when the argument contains commas, e.g.:
// #define DEFINE_VARIABLE(Type, Name) PREPROCESSOR_REMOVE_OPTIONAL_PARENS(Type) Name;
// DEFINE_VARIABLE(int, IntVar)                  // expands to: int IntVar;
// DEFINE_VARIABLE((TPair<int, float>), PairVar) // expands to: TPair<int, float> PairVar;
#define PREPROCESSOR_REMOVE_OPTIONAL_PARENS(...) PREPROCESSOR_JOIN_FIRST(PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL,PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL __VA_ARGS__)
#define PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL(...) PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL __VA_ARGS__
#define PREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPLPREPROCESSOR_REMOVE_OPTIONAL_PARENS_IMPL
