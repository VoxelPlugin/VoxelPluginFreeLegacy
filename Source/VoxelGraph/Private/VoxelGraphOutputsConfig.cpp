// Copyright 2020 Phyronnaz

#include "VoxelGraphOutputsConfig.h"
#include "CppTranslation/VoxelVariables.h"

TArray<FName> UVoxelGraphOutputsConfig::GetFloatOutputs() const
{
	TArray<FName> Result;
	for (auto& Output : Outputs)
	{
		if (Output.Category == EVoxelDataPinCategory::Float) 
		{
			Result.Add(Output.Name);
		}
	}
	return Result;
}

#if WITH_EDITOR
void UVoxelGraphOutputsConfig::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		// Iterate reverse so that last properties are changed
		for (int32 OutputIndex = Outputs.Num() - 1; OutputIndex >= 0 ; OutputIndex--)
		{
			auto& Output = Outputs[OutputIndex];

			if (!Output.GUID.IsValid())
			{
				Output.GUID = FGuid::NewGuid();
			}

			if (Output.Name.IsNone())
			{
				Output.Name = "CustomOutput";
			}

			Output.Name = *FVoxelVariable::SanitizeName(Output.Name.ToString());

			// Make sure the name is unique
			{
				int32 NameIndex = 1;
				bool bResultNameIndexValid;
				FName PotentialName;

				do
				{
					PotentialName = Output.Name;
					if (NameIndex != 1)
					{
						PotentialName.SetNumber(NameIndex);
					}

					bResultNameIndexValid = true;
					for (auto& OtherOutput : Outputs)
					{
						if (&OtherOutput != &Output && OtherOutput.Name == PotentialName)
						{
							bResultNameIndexValid = false;
							break;
						}
					}
					if (bResultNameIndexValid)
					{
						for (auto& OtherOutput : FVoxelGraphOutput::DefaultOutputs)
						{
							if (OtherOutput.Name == PotentialName)
							{
								bResultNameIndexValid = false;
								break;
							}
						}
					}

					NameIndex++;
				} while (!bResultNameIndexValid);

				Output.Name = PotentialName;
			}
		}
		
		OnPropertyChanged.Broadcast();
	}
}
#endif

void UVoxelGraphOutputsConfig::PostLoad()
{
	Super::PostLoad();

	for (auto& Output : Outputs)
	{
		if (!Output.GUID.IsValid())
		{
			Output.GUID = FGuid::NewGuid();
		}
	}
}