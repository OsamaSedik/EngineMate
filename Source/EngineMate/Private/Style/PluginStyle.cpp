
#include "EngineMate/Public/Style/PluginStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

FName FEngineMateStyle::StyleSetName("EngineMateStyle");
TSharedPtr<FSlateStyleSet> FEngineMateStyle::CreatedSlateStyleSet = nullptr;

void FEngineMateStyle::InitializeIcons()
{
	if (!CreatedSlateStyleSet.IsValid())
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);
	}
	
}

///EngineMate/Editor_Utility_Widges/Widgets/WBP_LevelAssist.WBP_LevelAssist
TSharedRef<FSlateStyleSet> FEngineMateStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));
	const FString IconDirectory = IPluginManager::Get().FindPlugin(TEXT("EngineMate"))->GetBaseDir() / "Resources";
	CustomStyleSet->SetContentRoot(IconDirectory);
	const FVector2D ContextIconSize(20.0f, 20.0f);
	CustomStyleSet->Set(FName("ContentBrowser.DeleteUnusedAssets"),
		new FSlateImageBrush(IconDirectory/"DeleteUnusedAssets.png",ContextIconSize));
	
	CustomStyleSet->Set(FName("ContentBrowser.DeleteEmptyFolders"),
		new FSlateImageBrush(IconDirectory/"DeleteEmptyFolders.png",ContextIconSize));
	
	CustomStyleSet->Set(FName("LevelEditor.LockActors"),
		new FSlateImageBrush(IconDirectory/"LockActors.png",ContextIconSize));
	
	CustomStyleSet->Set(FName("LevelEditor.UnLockActors"),
	new FSlateImageBrush(IconDirectory/"UnLockActors.png",ContextIconSize));
	
	const FVector2D ToolbarIconSize(32.0f, 32.0f);
	CustomStyleSet->Set(
	"EngineMate.OpenLevelAssistWindow",
	new FSlateImageBrush(IconDirectory / "LevelAssist.png", ToolbarIconSize)
	);

	CustomStyleSet->Set(
		"EngineMate.OpenPerformanceMonitorWindow",
		new FSlateImageBrush(IconDirectory / "Performance.png", ToolbarIconSize)
	);
	
	return CustomStyleSet;
}

void FEngineMateStyle::Shutdown()
{
	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
	}
}

FName FEngineMateStyle::GetStyleSetName()
{
	return StyleSetName;
}
