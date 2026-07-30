// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class UEditorActorSubsystem;

class FEngineMateModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
		
#pragma region  ContentBrowserMenuExtention
	
	TArray<FString> FolderPathsSelected;
	TArray<FAssetData> SelectedAssetData;
	
	void InitContentMenuExtension();
	TSharedRef<FExtender> CustomContentBrowserMenuExtender(const TArray<FString>& SelectedPaths);
	TSharedRef<FExtender> CustomAssetContentBrowserMenuExtender(const TArray<FAssetData>& SelectedAssets);
	void AddContentBrowserMenuEntry(FMenuBuilder& MenuBuilder);
	void AddAssetContextMenuEntry(FMenuBuilder& MenuBuilder);
	void OnDuplicateAssetClicked();
	void OnApplyNamingConventionClicked();
	void OnDeleteUnusedAssetMenuClicked();
	void OnDeleteUnusedAssetButtonClicked();
	void FixUpRedirectors();
	void GetAllFolders(const FString& RootPath, TArray<FString>& OutFolders);
	bool IsFolderTrulyEmpty(const FString& FolderPath);
	void OnDeleteUnusedFoldersButtonClicked();
#pragma endregion
	
#pragma region  LevelEditorMenuExtention
	
	void InitLevelEditorMenuExtension();
	
	TSharedRef<FExtender> CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors);
	
	void AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder);
	void OnLockActorSelectionButtonClicked();
	void OnUnlockActorSelectionButtonClicked();
#pragma endregion
	
#pragma region SelectionLock
	void InitCustomSelectionLock();
	void OnActorSelected(UObject* SelectedObject);
	void LockActorSelection(AActor* ActorToLock);
	void UnlockActorSelection(AActor* ActorToUnlock);
	bool CheckIsActorSelectionLocked(AActor* ActorToCheck);
#pragma endregion

	TWeakObjectPtr<UEditorActorSubsystem> EditorActorSubsystem;
	bool GetUEditorActorSubsystem();
	
#pragma region TOOLBAR_BUTON
	
	TSharedPtr<FUICommandList> PluginCommands;

	void InitializeToolbarButtons();
	void RegisterMenus();
	void OnOpenLevelAssistWindowClicked();
	void OnOpenPerformanceMonitorWindowClicked();

#pragma endregion
};
