// Copyright 2020 Phyronnaz

#include "VoxelUtilities/VoxelSystemUtilities.h"
#include "VoxelLogTable.h"

#include "Containers/Ticker.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

void FVoxelSystemUtilities::DelayedCall(TFunction<void()> Call, float Delay)
{
	check(IsInGameThread());
	
	FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([=](float)
	{
		Call();
		return false;
	}), Delay);
}

void FVoxelSystemUtilities::WriteTables(const FString& BaseName, const TArray<FTable>& Tables)
{
    TArray<FString> Lines;
    Lines.Add(R"(<!DOCTYPE html>)");
    Lines.Add(R"(<html lang="en-US">)");
    Lines.Add(R"(<head>)");
    Lines.Add(R"(<meta charset="utf-8">)");
    Lines.Add("<title>" + BaseName + "</title>");
    Lines.Add(R"(<link rel="stylesheet" href="table_style.css">)");
    Lines.Add(R"(</head>)");
    Lines.Add(R"(<script src="sorttable.js"></script>)");
	
    Lines.Add(R"(<body style="background: darkgray;">)");

    for (auto& It : Tables)
    {
	    Lines.Add("<h1>" + It.Title + "</h1>");
	    Lines.Append(It.Table->ToString(true));
    }
    
    Lines.Add(R"(</body>)");
    Lines.Add(R"(</html>)");

	const FString BasePath = FPaths::ProjectSavedDir() / "Voxel";
	const FString Path = BasePath / BaseName + "_" + FDateTime::Now().ToString() + ".html";
	const FString SourcePath = GetPlugin().GetBaseDir() / "Source" / "Voxel" / "Private" / "VoxelUtilities";

	IFileManager::Get().Copy(*(BasePath / "table_style.css"), *(SourcePath / "table_style.css"));
	IFileManager::Get().Copy(*(BasePath / "sorttable.js"), *(SourcePath / "sorttable.js"));
	
	ensure(FFileHelper::SaveStringArrayToFile(Lines, *Path));
	FPlatformProcess::LaunchURL(*Path, nullptr, nullptr);
}

IPlugin& FVoxelSystemUtilities::GetPlugin()
{
	return *IPluginManager::Get().FindPlugin(VOXEL_PLUGIN_NAME).ToSharedRef();
}