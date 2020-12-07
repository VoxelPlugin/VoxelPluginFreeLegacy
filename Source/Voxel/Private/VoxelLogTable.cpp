// Copyright 2020 Phyronnaz

#include "VoxelLogTable.h"

TArray<FString> FVoxelLogTable::ToString() const
{
	if (Rows.Num() == 0)
	{
		return {};
	}

	int32 MaxColumns = 0;
	for (auto& Row : Rows)
	{
		MaxColumns = FMath::Max(MaxColumns, Row.Columns.Num());
	}
	
	TArray<int32> ColumnsLengths;
	ColumnsLengths.SetNum(MaxColumns);
	for (auto& Row : Rows)
	{
		for (int32 Column = 0; Column < Row.Columns.Num(); Column++)
		{
			const FEntry& Entry = Row.Columns[Column];
			int32& ColumnLength = ColumnsLengths[Column];
			ColumnLength = FMath::Max3(ColumnLength, Entry.Title.Len(), Entry.Value.Len());
		}
	}
	
	TArray<FString> Lines;
	{
		const FRow& FirstRow = Rows[0];
		
		TArray<FString> ColumnsStrings;
		for (int32 Column = 0; Column < FirstRow.Columns.Num(); Column++)
		{
			const int32 Length = ColumnsLengths[Column] + 2;
			FString Text = FirstRow.Columns[Column].Title;

			ensure(Text.Len() < Length);
			Text = " " + Text + " ";
			Text = FString::ChrN((Length - Text.Len()) / 2, ' ') + Text;
			Text = Text.RightPad(Length);
			ensure(Text.Len() == Length);

			ColumnsStrings.Add(Text);
		}

		Lines.Add(FString::Join(ColumnsStrings, TEXT(";")));
	}
	
	for (auto& Row : Rows)
	{
		TArray<FString> ColumnsStrings;
		for (int32 Column = 0; Column < Row.Columns.Num(); Column++)
		{
			const int32 Length = ColumnsLengths[Column] + 2;
			FString Text = Row.Columns[Column].Value;

			ensure(Text.Len() < Length);
			Text = " " + Text + " ";
			Text = Text.LeftPad(Length);
			ensure(Text.Len() == Length);

			ColumnsStrings.Add(Text);
		}

		Lines.Add(FString::Join(ColumnsStrings, TEXT(";")));
	}

	return Lines;
}