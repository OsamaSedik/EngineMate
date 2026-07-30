#pragma once

#include "Styling/SlateStyle.h"

class FEngineMateStyle
{
	
public:
	
	static void InitializeIcons();
	static void Shutdown();
	static FName GetStyleSetName();
private:
	static FName StyleSetName;
	
	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet();
	static TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;
	
};
