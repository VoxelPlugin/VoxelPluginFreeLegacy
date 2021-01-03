// Copyright 2021 Phyronnaz

#include "VoxelLogTable.h"

TArray<FString> FVoxelLogTable::ToString(bool bHtml) const
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

	Lines.Add(R"(<table class="sortable">)");
	Lines.Add("\t<tr>");
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

		if (bHtml)
		{
			for (const FString& Column : ColumnsStrings)
			{
				Lines.Add(FString::Printf(TEXT("\t\t<th>%s</th>"), *Column));
			}
		}
		else
		{
			Lines.Add(FString::Join(ColumnsStrings, TEXT(";")));
		}
	}
	Lines.Add("\t</tr>");
	
	for (auto& Row : Rows)
	{
		Lines.Add("\t<tr>");
		
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
		

		if (bHtml)
		{
			for (int32 Column = 0; Column < Row.Columns.Num(); Column++)
			{
				Lines.Add(FString::Printf(TEXT("\t\t<td style=\"background-color: rgb(255 0 0 / %f%%);\" sorttable_customkey=\"%s\">%s</td>"),
					Row.Columns[Column].Percent / 1.5,
					*Row.Columns[Column].SortValue.ReplaceQuotesWithEscapedQuotes(), 
					*ColumnsStrings[Column]));
			}
		}
		else
		{
			Lines.Add(FString::Join(ColumnsStrings, TEXT(";")));
		}
		Lines.Add("\t</tr>");
	}
	Lines.Add("</table>");

	return Lines;
}