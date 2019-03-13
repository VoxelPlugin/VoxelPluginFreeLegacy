// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"

#if DO_CHECK
	#define ensureVoxel(    InExpression               ) (LIKELY(!!(InExpression)) || FVoxelCrashReporter::OptionallyLogFormattedEnsureMessageReturningFalse(UE4Asserts_Private::TrueOnFirstCallOnly([]{}), #InExpression, __FILE__, __LINE__, TEXT("")               ) || UE_ENSURE_BREAK_ONCE())
	#define ensureVoxelMsgf(InExpression, InFormat, ...) (LIKELY(!!(InExpression)) || FVoxelCrashReporter::OptionallyLogFormattedEnsureMessageReturningFalse(UE4Asserts_Private::TrueOnFirstCallOnly([]{}), #InExpression, __FILE__, __LINE__, InFormat, ##__VA_ARGS__) || UE_ENSURE_BREAK_ONCE())
#else
	#define ensureVoxel(           InExpression                ) (!!(InExpression))
	#define ensureVoxelMsgf(       InExpression, InFormat, ... ) (!!(InExpression))
#endif

class FVoxelCrashReporter
{
public:
	static void SubmitErrorReport(const TCHAR* InErrorHist);

	/**
	 * Logs an error if bLog is true, and returns false.  Takes a formatted string.
	 *
	 * @param	bLog	Log if true.
	 * @param	Expr	Code expression ANSI string (#code)
	 * @param	File	File name ANSI string (__FILE__)
	 * @param	Line	Line number (__LINE__)
	 * @param	FormattedMsg	Informative error message text with variable args
	 *
	 * @return false in all cases.
	 *
	 * Note: this crazy name is to ensure that the crash reporter recognizes it, which checks for functions in the callstack starting with 'EnsureNotFalse'.
	 */
	template <typename FmtType, typename... Types>
	static FORCEINLINE typename TEnableIf<TIsArrayOrRefOfType<FmtType, TCHAR>::Value, bool>::Type OptionallyLogFormattedEnsureMessageReturningFalse(bool bLog, const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const FmtType& FormattedMsg, Types... Args)
	{
		static_assert(TIsArrayOrRefOfType<FmtType, TCHAR>::Value, "Formatting string must be a TCHAR array.");
		static_assert(TAnd<TIsValidVariadicFunctionArg<Types>...>::Value, "Invalid argument(s) passed to ensureMsgf");

		return OptionallyLogFormattedEnsureMessageReturningFalseImpl(bLog, Expr, File, Line, FormattedMsg, Args...);
	}
	static bool VARARGS OptionallyLogFormattedEnsureMessageReturningFalseImpl(bool bLog, const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* FormattedMsg, ...);

	/**
	 * Called when an 'ensure' assertion fails; gathers stack data and generates and error report.
	 *
	 * @param	Expr	Code expression ANSI string (#code)
	 * @param	File	File name ANSI string (__FILE__)
	 * @param	Line	Line number (__LINE__)
	 * @param	Msg		Informative error message text
	 * 
	 * Don't change the name of this function, it's used to detect ensures by the crash reporter.
	 */
	static void EnsureFailed( const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* Msg );
};