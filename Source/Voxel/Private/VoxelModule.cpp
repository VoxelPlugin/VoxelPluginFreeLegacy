// Copyright 2020 Phyronnaz

#include "VoxelModule.h"

#include "VoxelMessages.h"
#include "IVoxelPool.h"
#include "VoxelTests.h"

#include "Containers/Ticker.h"
#include "Interfaces/IPluginManager.h"
#include "ShaderCore.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Misc/MessageDialog.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"

void FVoxelModule::StartupModule()
{
	LOG_VOXEL(Log, TEXT("VOXEL_DEBUG=%d"), VOXEL_DEBUG);
	
	if (VOXEL_DEBUG || !UE_BUILD_SHIPPING)
	{
		FVoxelTests::Test();
	}
	
	{
		const auto Plugin = IPluginManager::Get().FindPlugin(VOXEL_PLUGIN_NAME);

		// This is needed to correctly share content across Pro and Free
		FPackageName::UnRegisterMountPoint(TEXT("/") VOXEL_PLUGIN_NAME TEXT("/"), Plugin->GetContentDir());
		FPackageName::RegisterMountPoint("/Voxel/", Plugin->GetContentDir());

		const FString PluginBaseDir = Plugin.IsValid() ? FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir()) : "";

		const FString PluginShaderDir = FPaths::Combine(PluginBaseDir, TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/Voxel"), PluginShaderDir);

	}
		
	FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([=](float)
	{
		int32 VoxelPluginVersion = 0;
		GConfig->GetInt(TEXT("VoxelPlugin"), TEXT("VoxelPluginVersion"), VoxelPluginVersion, GEditorPerProjectIni);
		
		const auto OpenLink = [=](const FString& Url)
		{
			FString Error;
			FPlatformProcess::LaunchURL(*Url, nullptr, &Error);
			if (!Error.IsEmpty())
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Failed to open " + Url + "\n:" + Error));
			}
		};

		constexpr int32 LatestVoxelPluginVersion = 2;
		if (VoxelPluginVersion < LatestVoxelPluginVersion)
		{
			const auto Close = [=]()
			{
				GConfig->SetInt(TEXT("VoxelPlugin"), TEXT("VoxelPluginVersion"), LatestVoxelPluginVersion, GEditorPerProjectIni);
			};

			FVoxelMessages::FNotification Notification;
			Notification.UniqueId = OBJECT_LINE_ID();
			Notification.Message = "Voxel Plugin has been updated to 1.2!";
			Notification.Duration = 1e6f;
			Notification.OnClose = FSimpleDelegate::CreateLambda(Close);
			
			auto& Button = Notification.Buttons.Emplace_GetRef();
			Button.Text = "Show Release Notes";
			Button.Tooltip = "See the latest plugin release notes";
			Button.OnClick = FSimpleDelegate::CreateLambda([=]() 
			{
				OpenLink("https://releases.voxelplugin.com");
				Close();
			});
			
			FVoxelMessages::ShowNotification(Notification);
		}

		{
			FVoxelMessages::FNotification Notification;
			Notification.UniqueId = OBJECT_LINE_ID();
			Notification.Message = "Thanks for using Voxel Plugin Free!";
			Notification.Duration = 1e6f;

			{
				auto& Button = Notification.Buttons.Emplace_GetRef();
				Button.Text = "Docs";
				Button.Tooltip = "Open the plugin docs";
				Button.bCloseOnClick = false;
				Button.OnClick = FSimpleDelegate::CreateLambda([=]()
				{
					OpenLink("https://wiki.voxelplugin.com");
				});
			}

			{
				auto& Button = Notification.Buttons.Emplace_GetRef();
				Button.Text = "Discord";
				Button.Tooltip = "Open the plugin discord, where you can interact with an awesome community!";
				Button.bCloseOnClick = false;
				Button.OnClick = FSimpleDelegate::CreateLambda([=]()
				{
					OpenLink("https://discord.voxelplugin.com");
				});
			}

			{
				auto& Button = Notification.Buttons.Emplace_GetRef();
				Button.Text = "Pro";
				Button.Tooltip = "Open the Voxel Plugin Pro marketplace page";
				Button.bCloseOnClick = false;
				Button.OnClick = FSimpleDelegate::CreateLambda([=]()
				{
					OpenLink("https://pro.voxelplugin.com");
				});
			}
			
			FVoxelMessages::ShowNotification(Notification);
		}
		
		return false;
	}));
}

void FVoxelModule::ShutdownModule()
{
	IVoxelPool::Shutdown();
}

IMPLEMENT_MODULE(FVoxelModule, Voxel)