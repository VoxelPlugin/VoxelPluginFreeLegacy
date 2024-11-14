// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelDebugEditor.h"
#include "VoxelDebug.h"

#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelUtilities/VoxelTextureUtilities.h"
#include "VoxelUtilities/VoxelDistanceFieldUtilities.h"
#include "VoxelUtilities/VoxelConfigUtilities.h"

#include "Styling/SlateStyleRegistry.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SCheckBox.h"

#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Modules/ModuleManager.h"
#include "UObject/StrongObjectPtr.h"
#include "PropertyEditorModule.h"
#include "Async/Async.h"

class SVoxelDebug : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVoxelDebug)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args)
	{
		FVoxelDebug::GetDelegate<float>().Add(MakeWeakPtrDelegate(this, [=](FName Name, const FIntVector& Size, TArrayView<const float> Data)
		{
			AsyncTask(ENamedThreads::GameThread, [Name, Size, Data = TArray<float>(Data.GetData(), Data.Num())]()
			{
				auto& CustomData = *GetMutableDefault<UVoxelDebugParameters_CustomData>();
				auto& Array = CustomData.FloatData.FindOrAdd(Name);
				Array.Insert({ Data, Size }, 0);
				Array.SetNum(FMath::Min(Array.Num(), 100));
				CustomData.DataToDisplay.FindOrAdd(Name);
			});
		}));
		FVoxelDebug::GetDelegate<FVoxelValue>().Add(MakeWeakPtrDelegate(this, [=](FName Name, const FIntVector& Size, TArrayView<const FVoxelValue> Data)
		{
			AsyncTask(ENamedThreads::GameThread, [Name, Size, Data = TArray<FVoxelValue>(Data.GetData(), Data.Num())]()
			{
				auto& CustomData = *GetMutableDefault<UVoxelDebugParameters_CustomData>();
				auto& Array = CustomData.ValueData.FindOrAdd(Name);
				Array.Insert({ Data, Size }, 0);
				Array.SetNum(FMath::Min(Array.Num(), 100));
				CustomData.DataToDisplay.FindOrAdd(Name);
			});
		}));
		
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs;
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
		DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;

		Details = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
		const auto BaseDetails = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

		const auto Lambda = [this](const FPropertyChangedEvent& PropertyChangedEvent)
		{
			UpdateTexture();
		};
		Details->OnFinishedChangingProperties().AddLambda(Lambda);
		BaseDetails->OnFinishedChangingProperties().AddLambda(Lambda);

		BaseDetails->SetObject(GetMutableDefault<UVoxelDebugParameters_Base>());
		
		ChildSlot
		[
			SNew(SSplitter)
			+ SSplitter::Slot()
			.Value(1.f)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(&*Brush)
					]
					+ SOverlay::Slot()
					[
						SAssignNew(Grid, SGridPanel)
					]
				]
			]
			+ SSplitter::Slot()
			.Value(1.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					BaseDetails
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					Details.ToSharedRef()
				]
			]
		];

		GetMutableDefault<UVoxelDebugParameters_CustomData>()->PostEditChange.AddSP(this, &SVoxelDebug::UpdateTexture);
		
		FVoxelConfigUtilities::LoadConfig(GetMutableDefault<UVoxelDebugParameters_JumpFlood>(), "VoxelDebugParameters_JumpFlood");
		FVoxelConfigUtilities::LoadConfig(GetMutableDefault<UVoxelDebugParameters_CustomData>(), "VoxelDebugParameters_CustomData");
		UpdateTexture();
	}

	void UpdateTexture()
	{
		const auto GetSliceIndex = [](auto& Parameters, int32 X, int32 Y, const FIntVector& Size)
		{
			Parameters.Slice = FMath::Clamp(Parameters.Slice, 0, Size[int32(Parameters.SliceAxis)] - 1);
			
			FIntVector Position;
			Position[int32(Parameters.SliceAxis)] = Parameters.Slice;
			Position[(int32(Parameters.SliceAxis) + 1) % 3] = Parameters.bFlip ? Y : X;
			Position[(int32(Parameters.SliceAxis) + 2) % 3] = Parameters.bFlip ? X : Y;
			return Position.X + Size.X * Position.Y + Size.X * Size.Y * Position.Z;
		};
		const auto ProjectToTexture = [](auto& Parameters, auto X, auto Y, auto Z, auto& OutX, auto& OutY)
		{
			TArray<typename TDecay<decltype(X)>::Type, TFixedAllocator<3>> Array = { X, Y, Z };
			OutX = Array[(int32(Parameters.SliceAxis) + 1) % 3];
			OutY = Array[(int32(Parameters.SliceAxis) + 2) % 3];
			if (Parameters.bFlip)
			{
				Swap(OutX, OutY);
			}
		};
		const auto GetTextureSize = [&](auto& Parameters, const FIntVector& Size)
		{
			FIntPoint TextureSize;
			ProjectToTexture(Parameters, Size.X, Size.Y, Size.Z, TextureSize.X, TextureSize.Y);
			return TextureSize;
		};
		const auto GetColorIndex = [](const FIntPoint& TextureSize, int32 X, int32 Y)
		{
			return X + TextureSize.X * (TextureSize.Y - 1 - Y);
		};
		
		GetMutableDefault<UVoxelDebugParameters_Base>()->bUpdate = false;

		const auto Type = GetDefault<UVoxelDebugParameters_Base>()->DebugType;
		if (Type == EVoxelDebugType::JumpFlood)
		{
			auto& Parameters = *GetMutableDefault<UVoxelDebugParameters_JumpFlood>();
			Details->SetObject(&Parameters);
			FVoxelConfigUtilities::SaveConfig(&Parameters, "VoxelDebugParameters_JumpFlood");

			FIntVector Size = FIntVector::ZeroValue;

			TArray<FVector3f> SurfacePositions;
			TArray<float> Distances;

			if (Parameters.bUseMesh)
			{
				if (!Parameters.Mesh)
				{
					return;
				}

				if (Mesh != Parameters.Mesh)
				{
					UVoxelMeshImporterLibrary::CreateMeshDataFromStaticMesh(Parameters.Mesh, MeshData);
					Mesh = Parameters.Mesh;
				}

				const double StartTime = FPlatformTime::Seconds();

				FIntVector PositionOffset;
				int32 NumLeaks = 0;
				UVoxelMeshImporterLibrary::ConvertMeshToDistanceField(
					MeshData,
					Parameters.Transform,
					Parameters.MeshImporterSettings,
					Parameters.BoxExtension,
					Distances,
					SurfacePositions,
					Size,
					PositionOffset,
					NumLeaks,
					Parameters.bMultiThreaded,
					Parameters.Passes);

				const double EndTime = FPlatformTime::Seconds();

				Parameters.TimeInSeconds = EndTime - StartTime;
			}
			else
			{
				Size = FIntVector(Parameters.TextureSize);

				SurfacePositions.SetNumUninitialized(Size.X * Size.Y * Size.Z);
				Distances.SetNumUninitialized(Size.X * Size.Y * Size.Z);

				for (int32 X = 0; X < Size.X; X++)
				{
					for (int32 Y = 0; Y < Size.Y; Y++)
					{
						for (int32 Z = 0; Z < Size.Z; Z++)
						{
							FVector3f Position;
							float Distance;
							if (FVector3f(X - Size.X / 2.f, Y - Size.Y / 2.f, Z - Size.Z / 2.f).Size() < 16)
							{
								Position = FVector3f(X, Y, Z);
								Distance = 0.f;
							}
							else
							{
								Position = FVector3f(1e9);
								Distance = 1e9;
							}

							const int32 Index = X + Size.X * Y + Size.X * Size.Y * Z;
							SurfacePositions[Index] = Position;
							Distances[Index] = Distance;
						}
					}
				}

				const double StartTime = FPlatformTime::Seconds();

				FVoxelDistanceFieldUtilities::DownSample(Size, Distances, SurfacePositions, Parameters.Divisor, Parameters.bShrink);
				FVoxelDistanceFieldUtilities::JumpFlood(Size, SurfacePositions, Parameters.bMultiThreaded, Parameters.Passes);
				FVoxelDistanceFieldUtilities::GetDistancesFromSurfacePositions(Size, SurfacePositions, Distances);

				const double EndTime = FPlatformTime::Seconds();

				Parameters.TimeInSeconds = EndTime - StartTime;
			}
			
			Parameters.Size = Size;

			const auto TextureSize = GetTextureSize(Parameters, Size);
			
			TArray<FColor> Colors;
			Colors.Empty(TextureSize.X * TextureSize.Y);
			Colors.SetNumUninitialized(TextureSize.X * TextureSize.Y);

			for (int32 X = 0; X < TextureSize.X; X++)
			{
				for (int32 Y = 0; Y < TextureSize.Y; Y++)
				{
					const int32 ColorIndex = GetColorIndex(TextureSize, X, Y);

					const int32 Index = GetSliceIndex(Parameters, X, Y, Size);
					const FVector3f Position = SurfacePositions[Index];
					const float Distance = Distances[Index];

					if (Position.X >= 1e9)
					{
						Colors[ColorIndex] = FColor::Blue;
					}
					else
					{
						if (Parameters.bShowDistances)
						{
							Colors[ColorIndex] = FVoxelDistanceFieldUtilities::GetDistanceFieldColor(Distance / float(TextureSize.GetMax()));
						}
						else
						{
							float FX;
							float FY;
							ProjectToTexture(Parameters, Position.X / Size.X, Position.Y / Size.Y, Position.Z / Size.Z, FX, FY);
							Colors[ColorIndex] = FColor(
								FVoxelUtilities::FloatToUINT8(FX),
								FVoxelUtilities::FloatToUINT8(FY),
								0);
						}

					}
				}
			}

			SetColors(Colors, TextureSize);
			SetGrid({}, {});
		}
		else
		{
			auto& Parameters = *GetMutableDefault<UVoxelDebugParameters_CustomData>();
			Details->SetObject(&Parameters);
			FVoxelConfigUtilities::SaveConfig(&Parameters, "VoxelDebugParameters_CustomData");

			FName Name;
			for (auto& It : Parameters.DataToDisplay)
			{
				if (It.Value)
				{
					Name = It.Key;
					break;
				}
			}
			
			TArray<FColor> Colors;
			TArray<FString> Text;
			FIntVector Size { ForceInit };
			FIntPoint TextureSize { ForceInit };
			
			if (auto* ValueData = Parameters.ValueData.Find(Name))
			{
				Parameters.Frame = FMath::Clamp(Parameters.Frame, 0, ValueData->Num() - 1);
				auto& Data = (*ValueData)[Parameters.Frame].Data;
				Size = (*ValueData)[Parameters.Frame].Size;

				TextureSize = GetTextureSize(Parameters, Size);
				Colors.Empty(TextureSize.X * TextureSize.Y);
				Colors.SetNumUninitialized(TextureSize.X * TextureSize.Y);
				Text.Empty(TextureSize.X * TextureSize.Y);
				Text.SetNum(TextureSize.X * TextureSize.Y);
				
				for (int32 X = 0; X < TextureSize.X; X++)
				{
					for (int32 Y = 0; Y < TextureSize.Y; Y++)
					{
						const int32 Index = GetSliceIndex(Parameters, X, Y, Size);
						const int32 ColorIndex = GetColorIndex(TextureSize, X, Y);

						const FVoxelValue Value = Data[Index];

						if (Value.GetStorage() == 0)
						{
							Colors[ColorIndex] = FColor::Green;
						}
						else
						{
							Colors[ColorIndex] = FColor(
								FVoxelUtilities::FloatToUINT8(-Value.ToFloat()),
								0,
								FVoxelUtilities::FloatToUINT8(Value.ToFloat()));
						}
						Text[ColorIndex] = LexToString(Value.GetStorage());
					}
				}
			}
			if (auto* FloatData = Parameters.FloatData.Find(Name))
			{
				Parameters.Frame = FMath::Clamp(Parameters.Frame, 0, FloatData->Num() - 1);
				auto& Data = (*FloatData)[Parameters.Frame].Data;
				Size = (*FloatData)[Parameters.Frame].Size;

				TextureSize = GetTextureSize(Parameters, Size);
				Colors.Empty(TextureSize.X * TextureSize.Y);
				Colors.SetNumUninitialized(TextureSize.X * TextureSize.Y);
				Text.Empty(TextureSize.X * TextureSize.Y);
				Text.SetNum(TextureSize.X * TextureSize.Y);

				for (int32 X = 0; X < TextureSize.X; X++)
				{
					for (int32 Y = 0; Y < TextureSize.Y; Y++)
					{
						const int32 Index = GetSliceIndex(Parameters, X, Y, Size);
						const int32 ColorIndex = GetColorIndex(TextureSize, X, Y);

						const float Distance = Data[Index];
						
						if (Distance >= 1e9)
						{
							Colors[ColorIndex] = FColor::Green;
						}
						else
						{
							const float Value = Distance / float(TextureSize.GetMax());
							Colors[ColorIndex] = FColor(
								FVoxelUtilities::FloatToUINT8(-Value),
								0,
								FVoxelUtilities::FloatToUINT8(Value));
						}

						Text[ColorIndex] = LexToString(Distance);
					}
				}
			}

			Parameters.Size = Size;
			SetColors(Colors, TextureSize);
			SetGrid(Text, TextureSize);
		}
	}

	void SetColors(const TArray<FColor>& Colors, const FIntPoint& Size)
	{
		TObjectPtr<UTexture2D> Texture = TexturePtr.Get();
		if (Size.X == 0 || Size.Y == 0)
		{
			FVoxelTextureUtilities::UpdateColorTexture(Texture, { 1, 1 }, { FColor::Black });
		}
		else
		{
			FVoxelTextureUtilities::UpdateColorTexture(Texture, Size, Colors);
		}
		if (ensure(Texture))
		{
			Texture->Filter = TF_Nearest;
			TexturePtr.Reset(Texture);
		}
		Brush->SetImageSize(Size);
		Brush->SetResourceObject(Texture);
	}
	void SetGrid(const TArray<FString>& Data, const FIntPoint& Size) const
	{
		Grid->ClearChildren();
		if (Size.X == 0 || Size.Y == 0 || Data.Num() == 0)
		{
			return;
		}

		check(Data.Num() == Size.X * Size.Y);
		for (int32 X = 0; X < Size.X; X++)
		{
			for (int32 Y = 0; Y < Size.Y; Y++)
			{
				Grid->AddSlot(X, Y)
				[
					SNew(SBox)
					.WidthOverride(50)
					.HeightOverride(50)
					.Padding(FMargin(2))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFitX)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Data[X + Size.X * Y]))
						]
					]
				];
			}
		}
	}

private:
	const TSharedRef<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	TStrongObjectPtr<UTexture2D> TexturePtr;

	TSharedPtr<IDetailsView> Details;
	TSharedPtr<SGridPanel> Grid;
	
	TWeakObjectPtr<UStaticMesh> Mesh;
	FVoxelMeshImporterInputData MeshData;
};

void UVoxelDebugParameters_CustomData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	PostEditChange.Broadcast();
}

TSharedRef<SDockTab> FVoxelDebugEditor::CreateTab(const FSpawnTabArgs& Args)
{
	auto Tab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVoxelDebug)
		];
	Tab->SetTabIcon(FSlateStyleRegistry::FindSlateStyle("VoxelStyle")->GetBrush("VoxelIcon"));
	return Tab;
}
