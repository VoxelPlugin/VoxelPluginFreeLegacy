// Copyright 2019 Phyronnaz

#include "VoxelDebug/VoxelCrashReporter.h"
#include "VoxelGlobals.h"

#include "Misc/Paths.h"
#include "Misc/App.h"
#include "Misc/EngineVersion.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformOutputDevices.h"
#include "HAL/PlatformStackWalk.h"
#include "HAL/IConsoleManager.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "HttpModule.h"
#include "CoreGlobals.h"

static void CRASH()
{
	ensureVoxelMsgf(false, TEXT("RIP"));
}

FAutoConsoleCommand CmdTestVoxelCrashReporter(
	TEXT("voxel.crash"),
	TEXT(""),
	FConsoleCommandDelegate::CreateStatic(&CRASH)
	);

void FVoxelCrashReporter::SubmitErrorReport(const TCHAR* InErrorHist)
{
	//if (VOXELPLUGIN_VERSION != FString("0"))
	{
		const uint32 MAX_STRING_LEN = 256;

		TCHAR ReportDumpVersion[] = TEXT("3");

		FString Result;
		{
			TCHAR CompName[MAX_STRING_LEN];
			FCString::Strncpy(CompName, FPlatformProcess::ComputerName(), MAX_STRING_LEN);
			TCHAR UserName[MAX_STRING_LEN];
			FCString::Strncpy(UserName, FPlatformProcess::UserName(), MAX_STRING_LEN);
			TCHAR GameName[MAX_STRING_LEN];
			FCString::Strncpy(GameName, *FString::Printf(TEXT("%s %s"), *FApp::GetBranchName(), FApp::GetProjectName()), MAX_STRING_LEN);
			TCHAR PlatformName[MAX_STRING_LEN];
#if PLATFORM_64BITS
			FCString::Strncpy(PlatformName, TEXT("PC 64-bit"), MAX_STRING_LEN);
#else	//PLATFORM_64BITS
			FCString::Strncpy(PlatformName, TEXT("PC 32-bit"), MAX_STRING_LEN);
#endif	//PLATFORM_64BITS
			TCHAR CultureName[MAX_STRING_LEN];
			FCString::Strncpy(CultureName, *FInternationalization::Get().GetDefaultCulture()->GetName(), MAX_STRING_LEN);
			TCHAR SystemTime[MAX_STRING_LEN];
			FCString::Strncpy(SystemTime, *FDateTime::Now().ToString(), MAX_STRING_LEN);
			TCHAR EngineVersionStr[MAX_STRING_LEN];
			FCString::Strncpy(EngineVersionStr, *FEngineVersion::Current().ToString(), 256);
			TCHAR VoxelPluginVersionStr[MAX_STRING_LEN];
			FCString::Strncpy(VoxelPluginVersionStr, VOXELPLUGIN_VERSION, 256);

			TCHAR ChangelistVersionStr[MAX_STRING_LEN];
			int32 ChangelistFromCommandLine = 0;
			const bool bFoundAutomatedBenchMarkingChangelist = FParse::Value(FCommandLine::Get(), TEXT("-gABC="), ChangelistFromCommandLine);
			if (bFoundAutomatedBenchMarkingChangelist == true)
			{
				FCString::Strncpy(ChangelistVersionStr, *FString::FromInt(ChangelistFromCommandLine), MAX_STRING_LEN);
			}
			// we are not passing in the changelist to use so use the one that was stored in the ObjectVersion
			else
			{
				FCString::Strncpy(ChangelistVersionStr, *FString::FromInt(FEngineVersion::Current().GetChangelist()), MAX_STRING_LEN);
			}

			TCHAR CmdLine[2048];
			FCString::Strncpy(CmdLine, FCommandLine::Get(), ARRAY_COUNT(CmdLine));
			TCHAR BaseDir[260];
			FCString::Strncpy(BaseDir, FPlatformProcess::BaseDir(), ARRAY_COUNT(BaseDir));
			TCHAR separator = '\n';

			TCHAR EngineMode[MAX_STRING_LEN];
			if (IsRunningCommandlet())
			{
				FCString::Strncpy(EngineMode, TEXT("Commandlet"), MAX_STRING_LEN);
			}
			else if (GIsEditor)
			{
				FCString::Strncpy(EngineMode, TEXT("Editor"), MAX_STRING_LEN);
			}
			else if (IsRunningDedicatedServer())
			{
				FCString::Strncpy(EngineMode, TEXT("Server"), MAX_STRING_LEN);
			}
			else
			{
				FCString::Strncpy(EngineMode, TEXT("Game"), MAX_STRING_LEN);
			}

			//build the report dump file
			Result.AppendChars(ReportDumpVersion, FCString::Strlen(ReportDumpVersion) * 1);
			Result.AppendChars(&separator, 1);
			Result.AppendChars(CompName, FCString::Strlen(CompName));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(UserName, FCString::Strlen(UserName));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(GameName, FCString::Strlen(GameName));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(PlatformName, FCString::Strlen(PlatformName));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(CultureName, FCString::Strlen(CultureName));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(SystemTime, FCString::Strlen(SystemTime));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(EngineVersionStr, FCString::Strlen(EngineVersionStr));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(ChangelistVersionStr, FCString::Strlen(ChangelistVersionStr));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(CmdLine, FCString::Strlen(CmdLine));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(BaseDir, FCString::Strlen(BaseDir));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(EngineMode, FCString::Strlen(EngineMode));
			Result.AppendChars(&separator, 1);
			Result.AppendChars(&separator, 1);
			
			TCHAR* NonConstErrorHist = const_cast<TCHAR*>(InErrorHist);
			Result.AppendChars(NonConstErrorHist, FCString::Strlen(NonConstErrorHist));

			
			auto Request = FHttpModule::Get().CreateRequest();
			Request->OnHeaderReceived().BindLambda([](FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue)
			{

			});
			Request->SetVerb(TEXT("POST"));
			Request->SetURL("http://127.0.0.1:8080");
			Request->SetHeader(TEXT("Content-Type"), TEXT("text/plain; charset=UTF-8"));
			Request->SetContentAsString(Result);
			Request->ProcessRequest();
		}
	}
}

#define FILE_LINE_DESC TEXT(" [File:%s] [Line: %i] ")
#define CALLSTACK_IGNOREDEPTH 0

/**
 *	Prints error to the debug output, 
 *	prompts for the remote debugging if there is not debugger, breaks into the debugger 
 *	and copies the error into the global error message.
 */
void StaticFailDebug(const TCHAR* Error, const ANSICHAR* File, int32 Line, const TCHAR* Description, bool bIsEnsure)
{
	TCHAR DescriptionAndTrace[4096];

	FCString::Strncpy(DescriptionAndTrace, Description, ARRAY_COUNT(DescriptionAndTrace) - 1);

	// some platforms (Windows, Mac, Linux) generate this themselves by throwing an exception and capturing
	// the backtrace later on
	if (FPlatformProperties::AllowsCallStackDumpDuringAssert() && bIsEnsure == false)
	{
		ANSICHAR StackTrace[4096];
		if (StackTrace != NULL)
		{
			StackTrace[0] = 0;
			FPlatformStackWalk::StackWalkAndDump(StackTrace, ARRAY_COUNT(StackTrace), CALLSTACK_IGNOREDEPTH);

			FCString::Strncat(DescriptionAndTrace, TEXT("\n"), ARRAY_COUNT(DescriptionAndTrace) - 1);
			FCString::Strncat(DescriptionAndTrace, ANSI_TO_TCHAR(StackTrace), ARRAY_COUNT(DescriptionAndTrace) - 1);
		}
	}

	// Copy the detailed error into the error message.
	FCString::Snprintf(GErrorMessage, ARRAY_COUNT(GErrorMessage), TEXT("%s") FILE_LINE_DESC TEXT("\n%s\n"), Error, ANSI_TO_TCHAR(File), Line, DescriptionAndTrace);

	// Copy the error message to the error history.
	FCString::Strncpy(GErrorHist, GErrorMessage, ARRAY_COUNT(GErrorHist));
	FCString::Strncat(GErrorHist, TEXT("\r\n\r\n"), ARRAY_COUNT(GErrorHist));
}

void FVoxelCrashReporter::EnsureFailed(const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* Msg)
{
	// Print out the blueprint callstack
	PrintScriptCallstack();

	// Print initial debug message for this error
	TCHAR ErrorString[MAX_SPRINTF];
	FCString::Sprintf(ErrorString, TEXT("Ensure condition failed: %s"), ANSI_TO_TCHAR(Expr));

	StaticFailDebug(ErrorString, File, Line, Msg, true);
	
	// Is there a debugger attached?  If not we'll submit an error report.
	if (FPlatformMisc::IsDebuggerPresent())
	{
#if !NO_LOGGING
		UE_LOG(LogOutputDevice, Error, TEXT("%s") FILE_LINE_DESC TEXT("\n%s\n"), ErrorString, ANSI_TO_TCHAR(File), Line, Msg);
#endif
	}
	else
	{
		// Walk the stack and dump it to the allocated memory.
		const SIZE_T StackTraceSize = 65535;
		ANSICHAR* StackTrace = (ANSICHAR*)FMemory::SystemMalloc(StackTraceSize);
		if (StackTrace)
		{
			{
				StackTrace[0] = 0;
				FPlatformStackWalk::StackWalkAndDumpEx(StackTrace, StackTraceSize, CALLSTACK_IGNOREDEPTH, FGenericPlatformStackWalk::EStackWalkFlags::FlagsUsedWhenHandlingEnsure);
			}

			// Create a final string that we'll output to the log (and error history buffer)
			TCHAR ErrorMsg[16384];
			FCString::Snprintf(ErrorMsg, ARRAY_COUNT(ErrorMsg), TEXT("Ensure condition failed: %s [File:%s] [Line: %i]") LINE_TERMINATOR TEXT("%s") LINE_TERMINATOR TEXT("Stack: ") LINE_TERMINATOR, ANSI_TO_TCHAR(Expr), ANSI_TO_TCHAR(File), Line, Msg);

			// Also append the stack trace
			FCString::Strncat(ErrorMsg, ANSI_TO_TCHAR(StackTrace), ARRAY_COUNT(ErrorMsg) - 1);
			FMemory::SystemFree(StackTrace);

			// Dump the error and flush the log.
#if !NO_LOGGING
			FDebug::LogFormattedMessageWithCallstack(LogOutputDevice.GetCategoryName(), __FILE__, __LINE__, TEXT("=== Handled ensure: ==="), ErrorMsg, ELogVerbosity::Error);
#endif
			GLog->Flush();
			SubmitErrorReport(ErrorMsg);
		}
	}
}

bool VARARGS FVoxelCrashReporter::OptionallyLogFormattedEnsureMessageReturningFalseImpl( bool bLog, const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* FormattedMsg, ... )
{
	if (bLog)
	{
		const int32 TempStrSize = 4096;
		TCHAR TempStr[ TempStrSize ];
		GET_VARARGS( TempStr, TempStrSize, TempStrSize - 1, FormattedMsg, FormattedMsg );
		EnsureFailed( Expr, File, Line, TempStr );
	}
	
	return false;
}