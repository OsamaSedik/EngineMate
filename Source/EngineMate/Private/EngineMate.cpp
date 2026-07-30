// Copyright Epic Games, Inc. All Rights Reserved.

#include "EngineMate.h"
#include "EngineMate/Public/Style/PluginStyle.h"
#include "AssetToolsModule.h"
#include "ContentBrowserDelegates.h"
#include "ContentBrowserModule.h"
#include "EngineMate/Public/Misc/DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "LevelEditor.h"
#include "Engine/Selection.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Commands/FEngineMateCommands.h"
#include "ToolMenus.h"
#include "AssetActions/AutomatedAssetActions.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Styling/AppStyle.h"
#include "ScopedTransaction.h"

class SDuplicateAssetDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDuplicateAssetDialog) {}
	SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
	SLATE_END_ARGS()

	int32 DuplicateCount = 2;
	bool bConfirmed = false;
	TSharedPtr<SWindow> ParentWindow;

	void Construct(const FArguments& InArgs)
	{
		ParentWindow = InArgs._ParentWindow;

		ChildSlot
		[
			SNew(SBorder)
			.Padding(16)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 12)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("How many duplicates would you like to create? (2 to 10)")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 16)
				[
					SNew(SSpinBox<int32>)
					.Value_Lambda([this]() { return DuplicateCount; })
					.OnValueChanged_Lambda([this](int32 NewValue) { DuplicateCount = FMath::Clamp(NewValue, 2, 10); })
					.MinValue(2)
					.MaxValue(10)
					.MinSliderValue(2)
					.MaxSliderValue(10)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0, 0, 8, 0)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Duplicate")))
						.OnClicked_Lambda([this]()
						{
							bConfirmed = true;
							if (ParentWindow.IsValid())
							{
								ParentWindow->RequestDestroyWindow();
							}
							return FReply::Handled();
						})
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Cancel")))
						.OnClicked_Lambda([this]()
						{
							bConfirmed = false;
							if (ParentWindow.IsValid())
							{
								ParentWindow->RequestDestroyWindow();
							}
							return FReply::Handled();
						})
					]
				]
			]
		];
	}
};

#define LOCTEXT_NAMESPACE "FEngineMateModule"

void FEngineMateModule::StartupModule()
{
	FEngineMateStyle::InitializeIcons();
	InitContentMenuExtension();
	InitLevelEditorMenuExtension();
	InitCustomSelectionLock();
	InitializeToolbarButtons();
}

void FEngineMateModule::ShutdownModule()
{
	FEngineMateStyle::Shutdown();
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

#pragma region  ContentBrowserMenuExtention

void FEngineMateModule::InitContentMenuExtension()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	// Folder context menu extender
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	FContentBrowserMenuExtender_SelectedPaths CustomContentBrowserMenuDelegate;
	CustomContentBrowserMenuDelegate.BindRaw(this, &FEngineMateModule::CustomContentBrowserMenuExtender);
	ContentBrowserMenuExtenders.Add(CustomContentBrowserMenuDelegate);

	// Asset context menu extender (right-clicking assets)
	TArray<FContentBrowserMenuExtender_SelectedAssets>& AssetMenuExtenders = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	FContentBrowserMenuExtender_SelectedAssets CustomAssetMenuDelegate;
	CustomAssetMenuDelegate.BindRaw(this, &FEngineMateModule::CustomAssetContentBrowserMenuExtender);
	AssetMenuExtenders.Add(CustomAssetMenuDelegate);
}

TSharedRef<FExtender> FEngineMateModule::CustomContentBrowserMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender);
	
	if (SelectedPaths.Num() > 0)
	{
		FolderPathsSelected = SelectedPaths;
		MenuExtender->AddMenuExtension(
			FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FEngineMateModule::AddContentBrowserMenuEntry)
		);
	}

	return MenuExtender;
}

TSharedRef<FExtender> FEngineMateModule::CustomAssetContentBrowserMenuExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender);
	
	if (SelectedAssets.Num() > 0)
	{
		SelectedAssetData = SelectedAssets;
		MenuExtender->AddMenuExtension(
			FName("GetAssetActions"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FEngineMateModule::AddAssetContextMenuEntry)
		);
	}

	return MenuExtender;
}

void FEngineMateModule::AddContentBrowserMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely Delete All Unused Assets Under Folder")),
		FSlateIcon(FEngineMateStyle::GetStyleSetName(),FName("ContentBrowser.DeleteUnusedAssets")),
		FExecuteAction::CreateRaw(this,&FEngineMateModule::OnDeleteUnusedAssetButtonClicked)
		);
	
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely Delete All Empty Folders")),
		FSlateIcon(FEngineMateStyle::GetStyleSetName(),"ContentBrowser.DeleteEmptyFolders"),
		FExecuteAction::CreateRaw(this,&FEngineMateModule::OnDeleteUnusedFoldersButtonClicked)
		);
}

void FEngineMateModule::AddAssetContextMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("ScriptedAssetActions", LOCTEXT("ScriptedAssetActions", "Scripted Asset Actions"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DuplicateAsset", "Duplicate Asset"),
			LOCTEXT("DuplicateAssetTooltip", "Duplicate selected asset multiple times"),
			FSlateIcon(),
			FExecuteAction::CreateRaw(this, &FEngineMateModule::OnDuplicateAssetClicked)
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("ApplyNamingConvention", "Apply Naming Convention"),
			LOCTEXT("ApplyNamingConventionTooltip", "Automatically apply prefix naming convention to selected assets"),
			FSlateIcon(),
			FExecuteAction::CreateRaw(this, &FEngineMateModule::OnApplyNamingConventionClicked)
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteUnusedAssets", "Delete Unused Assets"),
			LOCTEXT("DeleteUnusedAssetsTooltip", "Safely delete selected assets if they have no references"),
			FSlateIcon(),
			FExecuteAction::CreateRaw(this, &FEngineMateModule::OnDeleteUnusedAssetMenuClicked)
		);
	}
	MenuBuilder.EndSection();
}

void FEngineMateModule::OnDuplicateAssetClicked()
{
	TSharedRef<SWindow> ModalWindow = SNew(SWindow)
		.Title(FText::FromString(TEXT("Duplicate Asset")))
		.ClientSize(FVector2D(360, 140))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		.IsTopmostWindow(true);

	TSharedPtr<SDuplicateAssetDialog> DialogWidget;
	ModalWindow->SetContent(
		SAssignNew(DialogWidget, SDuplicateAssetDialog)
		.ParentWindow(ModalWindow)
	);

	GEditor->EditorAddModalWindow(ModalWindow);

	if (DialogWidget.IsValid() && DialogWidget->bConfirmed)
	{
		UAutomatedAssetActions* Actions = NewObject<UAutomatedAssetActions>();
		Actions->DuplicateAsset(DialogWidget->DuplicateCount, SelectedAssetData);
	}
}

void FEngineMateModule::OnApplyNamingConventionClicked()
{
	TArray<UObject*> SelectedObjects;
	for (const FAssetData& AssetData : SelectedAssetData)
	{
		if (UObject* Asset = AssetData.GetAsset())
		{
			SelectedObjects.Add(Asset);
		}
	}
	UAutomatedAssetActions* Actions = NewObject<UAutomatedAssetActions>();
	Actions->ApplyNamingConvention(SelectedObjects);
}

void FEngineMateModule::OnDeleteUnusedAssetMenuClicked()
{
	UAutomatedAssetActions* Actions = NewObject<UAutomatedAssetActions>();
	Actions->DeleteUnusedAssets(SelectedAssetData);
}

void FEngineMateModule::OnDeleteUnusedAssetButtonClicked()
{
	// delete one folder per action
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHelper::ShowMessageDialog(EAppMsgType::Ok,TEXT("You Can Only Apply This Action On a Single Folder"),false);
		return;
	}
	
	// get hold for the all assets path from the selected folder  
	TArray<FString> AssetsPathNamesArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	if (AssetsPathNamesArray.Num() == 0)
	{
		DebugHelper::ShowMessageDialog(EAppMsgType::Ok,TEXT("No Assets Found Under Selected Folder"),false);
		return;
	}
	
	// show confirmation for the user 
	const EAppReturnType::Type ConfirmationResult = DebugHelper::ShowMessageDialog(EAppMsgType::YesNo,TEXT("a Total Of ") + 
		FString::FromInt(AssetsPathNamesArray.Num()) + TEXT("Found.. \n Would You Like To Delete Them ?"),false);
		
	if (ConfirmationResult == EAppReturnType::No) return; // exit if no 
	
	FixUpRedirectors();
	
	// array for all Unused assets' data. will be filled if references is empty 
	TArray<FAssetData> UnusedAssetDataArray;

	// range loop for all assets paths in the selected folder 
	for (const FString& AssetsPathName : AssetsPathNamesArray)
	{
		// don't delete any asset in these folder ... Engine Will crash
		if(AssetsPathName.Contains(TEXT("Collections")) ||
			AssetsPathName.Contains(TEXT("Developers")))
		{
			continue;
		}
		
		// still continue until found valid asset 
		if (!UEditorAssetLibrary::DoesAssetExist(AssetsPathName)) continue;
		
		// get hold for the package references for the asset path name -> will return an array of references  
		TArray<FString> AssetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetsPathName);
		
		// if there is no references count by this asset add them to the Unused asset array 
		if (AssetReferences.Num() == 0)
		{
			const FAssetData UnusedAsset = UEditorAssetLibrary::FindAssetData(AssetsPathName);
			UnusedAssetDataArray.Add(UnusedAsset);
		}
	}
	
	// delete all Unused assets from the array 
	if (UnusedAssetDataArray.Num()>0)
	{
		ObjectTools::DeleteAssets(UnusedAssetDataArray);
	}
	
	// show dialog message 
	else
	{
		DebugHelper::ShowMessageDialog(EAppMsgType::Ok,TEXT("No Unused Assets Found Under Selected Folder"),false);
	}
}

void FEngineMateModule::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorToFixArray;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassPaths.Emplace("ObjectRedirector");
	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter,OutRedirectors);
	for (const FAssetData& AssetData : OutRedirectors)
	{
		UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(AssetData.GetAsset());
		if (RedirectorToFix)
		{
			RedirectorToFixArray.Add(RedirectorToFix);
		}
	}
	
	if (RedirectorToFixArray.Num()>0)
	{
		const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		AssetToolsModule.Get().FixupReferencers(RedirectorToFixArray);
	}
	
}

void FEngineMateModule::GetAllFolders(const FString& RootPath, TArray<FString>& OutFolders)
{
	const FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FString> SubPaths;
	AssetRegistry.GetSubPaths(*RootPath, SubPaths, true);

	OutFolders = SubPaths;
}

bool FEngineMateModule::IsFolderTrulyEmpty(const FString& FolderPath)
{
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Convert to package path (/Game/...)
	const FName PackagePath(*FolderPath);

	// Get all assets recursively
	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByPath(PackagePath, Assets, true);

	return Assets.Num() == 0;
}


void FEngineMateModule::OnDeleteUnusedFoldersButtonClicked()
{
	FixUpRedirectors();
	TArray<FString> FolderPathsArray;
	GetAllFolders(FolderPathsSelected[0], FolderPathsArray);
	uint32 Counter = 0;
	FString EmptyFolderPathNames;
	TArray<FString> EmptyFolderPathsArray;
	for (const FString& FolderPath : FolderPathsArray)
	{
		// don't delete any asset in these folder ... Engine Will crash
		if(FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("Developers"))||
			FolderPath.Contains(TEXT("__ExternalActors__"))||
			FolderPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (IsFolderTrulyEmpty(FolderPath))
		{
			EmptyFolderPathNames.Append(FolderPath + TEXT("\n"));
			EmptyFolderPathsArray.Add(FolderPath);
		}
	}
	
	if (EmptyFolderPathsArray.Num() == 0)
	{
		DebugHelper::ShowMessageDialog(EAppMsgType::Ok,TEXT("No Empty Folders Found Under Selected Folder"),false);
		return;
	}
	 EmptyFolderPathNames = FString::Join(EmptyFolderPathsArray, TEXT("\n"));

	const EAppReturnType::Type ConfirmationResult =
		DebugHelper::ShowMessageDialog(
			EAppMsgType::YesNo,
			FString::Printf(
				TEXT("Empty Folders Found (%d):\n\n%s\n\nDelete these folders?"),
				EmptyFolderPathsArray.Num(),
				*EmptyFolderPathNames
			),
			false
		);
	if (ConfirmationResult == EAppReturnType::No) return;
	
	for (const FString& EmptyFolderPath : EmptyFolderPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			++Counter;
		}
		else
		{
			DebugHelper::PrintDebugScreen(TEXT("Failed To Delete Empty " + EmptyFolderPath),FColor::Red,5.0f) ;
		}
	}
	
	if (Counter>0)
	{
		DebugHelper::ShowNotifyInfo(TEXT("Successfully Delete ") + FString::FromInt(Counter) + TEXT(" Folders"));
	}
}

#pragma endregion

#pragma region  LevelEditorMenuExtention
void FEngineMateModule::InitLevelEditorMenuExtension()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelEditorMenuExtenders = 
			LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
	LevelEditorMenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::
		    CreateRaw(this,&FEngineMateModule::CustomLevelEditorMenuExtender));
}

TSharedRef<FExtender> FEngineMateModule::CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList,const TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuExtension(FName("ActorOptions"),
		EExtensionHook::Before,
		UICommandList,
		FMenuExtensionDelegate::CreateRaw(this,&FEngineMateModule::AddLevelEditorMenuEntry));
	return MenuExtender;
}

void FEngineMateModule::AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Lock Actor Selection")),
		FText::FromString(TEXT("Prevent Actor From Being Selected In the Editor")),
		FSlateIcon(FEngineMateStyle::GetStyleSetName(),FName("LevelEditor.LockActors")),
		FExecuteAction::CreateRaw(this,&FEngineMateModule::OnLockActorSelectionButtonClicked)
		);
	
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Unlock Actor Selection")),
		FText::FromString(TEXT("Remove the selection constraint on locked actors")),
		FSlateIcon(FEngineMateStyle::GetStyleSetName(),FName("LevelEditor.UnLockActors")),
		FExecuteAction::CreateRaw(this,&FEngineMateModule::OnUnlockActorSelectionButtonClicked)
		);
}

void FEngineMateModule::OnLockActorSelectionButtonClicked()
{
	if (!GetUEditorActorSubsystem()) return;
	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	if (SelectedActors.IsEmpty())
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Actors Selected."));
		return;
	}

	FScopedTransaction Transaction(LOCTEXT("LockActorSelection", "Lock Actor Selection"));

	FString CurrentActorLockName = TEXT("Lock Selection For :  ");
	for (AActor* Actor : SelectedActors)
	{
		if (!Actor) continue;
		LockActorSelection(Actor);
		CurrentActorLockName.Append(TEXT("\n") + Actor->GetActorLabel());
		EditorActorSubsystem->SetActorSelectionState(Actor,false);
	}
	DebugHelper::ShowNotifyInfo(CurrentActorLockName);
}

void FEngineMateModule::OnUnlockActorSelectionButtonClicked()
{
	if (!GetUEditorActorSubsystem()) return;

	FScopedTransaction Transaction(LOCTEXT("UnlockActorSelection", "Unlock Actor Selection"));

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	TArray<AActor*> TargetActorsToUnlock;

	if (SelectedActors.Num() > 0)
	{
		for (AActor* Actor : SelectedActors)
		{
			if (Actor && CheckIsActorSelectionLocked(Actor))
			{
				TargetActorsToUnlock.Add(Actor);
			}
		}
	}

	if (TargetActorsToUnlock.Num() == 0)
	{
		TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();
		for (AActor* Actor : AllLevelActors)
		{
			if (Actor && CheckIsActorSelectionLocked(Actor))
			{
				TargetActorsToUnlock.Add(Actor);
			}
		}
	}

	if (TargetActorsToUnlock.IsEmpty())
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Locked Actors Found In Level."));
		return;
	}

	FString CurrentActorLockName = TEXT("Unlock Actor Selection For :  ");
	for (AActor* Actor : TargetActorsToUnlock)
	{
		UnlockActorSelection(Actor);
		CurrentActorLockName.Append(TEXT("\n") + Actor->GetActorLabel());
	}
	DebugHelper::ShowNotifyInfo(CurrentActorLockName);
}

#pragma endregion

#pragma region SelectionLock
void FEngineMateModule::InitCustomSelectionLock()
{
	const USelection* UserSelection =  GEditor->GetSelectedActors();
	UserSelection->SelectionChangedEvent.AddRaw(this,&FEngineMateModule::OnActorSelected);
}

void FEngineMateModule::OnActorSelected(UObject* SelectedObject)
{
	if (!GetUEditorActorSubsystem()) return;

	USelection* Selection = Cast<USelection>(SelectedObject);
	if (!Selection) return;

	TArray<AActor*> SelectedActors;
	Selection->GetSelectedObjects<AActor>(SelectedActors);

	for (AActor* Actor : SelectedActors)
	{
		if (!Actor) continue;

		if (CheckIsActorSelectionLocked(Actor))
		{
			Actor->SetLockLocation(true);
		}
	}
}

void FEngineMateModule::LockActorSelection(AActor* ActorToLock)
{
	if (!ActorToLock) return;
	if (!ActorToLock->ActorHasTag(FName("Locked")))
	{
		ActorToLock->Modify();
		ActorToLock->Tags.Add(FName("Locked"));
		ActorToLock->SetLockLocation(true);
	}
}

void FEngineMateModule::UnlockActorSelection(AActor* ActorToUnlock)
{
	if (!ActorToUnlock) return;
	if (ActorToUnlock->ActorHasTag(FName("Locked")))
	{
		ActorToUnlock->Modify();
		ActorToUnlock->Tags.Remove(FName("Locked"));
		ActorToUnlock->SetLockLocation(false);
	}
}

bool FEngineMateModule::CheckIsActorSelectionLocked(AActor* ActorToCheck)
{
	if (!ActorToCheck) return false;
	return ActorToCheck->ActorHasTag(FName("Locked"));
}

bool FEngineMateModule::GetUEditorActorSubsystem() 
{
	if (!EditorActorSubsystem.IsValid())
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return EditorActorSubsystem.IsValid();
}


#pragma endregion


#pragma region TOOLBAR_BUTON

void FEngineMateModule::InitializeToolbarButtons()
{
	FEngineMateCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);
	
	PluginCommands->MapAction(
		FEngineMateCommands::Get().OpenLevelAssistWindow,
		FExecuteAction::CreateRaw(this, &FEngineMateModule::OnOpenLevelAssistWindowClicked),
		FCanExecuteAction()
	);
	
	PluginCommands->MapAction(
		FEngineMateCommands::Get().OpenPerformanceMonitorWindow,
		FExecuteAction::CreateRaw(this, &FEngineMateModule::OnOpenPerformanceMonitorWindowClicked),
		FCanExecuteAction()
	);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FEngineMateModule::RegisterMenus)
	);
}

void FEngineMateModule::RegisterMenus()
{
	UE_LOG(LogTemp, Warning, TEXT("RegisterMenus CALLED"));

	FToolMenuOwnerScoped OwnerScoped(this);
	
	UToolMenu* ToolbarMenu =
		UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");

	FToolMenuSection& Section = ToolbarMenu->AddSection("EngineMateSection");

	// Level Assist Window Button 
	{
		FToolMenuEntry Entry = FToolMenuEntry::InitToolBarButton(
			FEngineMateCommands::Get().OpenLevelAssistWindow
		);

		Entry.SetCommandList(PluginCommands);
		Section.AddEntry(Entry);
	}

	// Performance Monitor Window Button 
	{
		FToolMenuEntry Entry = FToolMenuEntry::InitToolBarButton(
			FEngineMateCommands::Get().OpenPerformanceMonitorWindow
		);

		Entry.SetCommandList(PluginCommands);
		Section.AddEntry(Entry);
	}
}

void FEngineMateModule::OnOpenLevelAssistWindowClicked()
{
	UEditorUtilitySubsystem* Subsystem =
		GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();

	if (!Subsystem) return;

	const FString Path =
		TEXT("/EngineMate/Editor_Utility_Widges/Widgets/WBP_LevelAssist.WBP_LevelAssist");

	UEditorUtilityWidgetBlueprint* Widget =
		LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, *Path);

	if (!Widget)
	{
		UE_LOG(LogTemp, Error, TEXT("Widget not found"));
		return;
	}

	Subsystem->SpawnAndRegisterTab(Widget);
}

void FEngineMateModule::OnOpenPerformanceMonitorWindowClicked()
{
	UEditorUtilitySubsystem* Subsystem =
		GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();

	if (!Subsystem) return;

	const FString Path =
		TEXT("/EngineMate/Editor_Utility_Widges/Widgets/WBP_Performance_Monitor.WBP_Performance_Monitor");

	UEditorUtilityWidgetBlueprint* Widget =
		LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, *Path);

	if (!Widget)
	{
		UE_LOG(LogTemp, Error, TEXT("Widget not found"));
		return;
	}

	Subsystem->SpawnAndRegisterTab(Widget);
}


#pragma endregion
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEngineMateModule, EngineMate)