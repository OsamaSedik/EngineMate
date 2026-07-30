

#include "AssetActions/AutomatedAssetActions.h"
#include "AssetToolsModule.h"
#include "EngineMate/Public/Misc/DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Settings/EngineMateSettings.h"

void UAutomatedAssetActions::FixUpReDirectors()
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
	
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().FixupReferencers(RedirectorToFixArray);
}

void UAutomatedAssetActions::DuplicateAsset(int32 NumOdDuplicates, TArray<FAssetData> InSelectedAssets)
{
	if (NumOdDuplicates <= 0)
	{
		DebugHelper::ShowMessageDialog(EAppMsgType::Type::Ok,TEXT("Pleas Enter a Valid Number"),true);
		return;
	}
	if (NumOdDuplicates > 10)
	{
		DebugHelper::ShowMessageDialog(EAppMsgType::Type::Ok,TEXT("Pleas Enter a Number Between 2 to 10 "),true);
		return;
	}
	TArray<FAssetData> SelectedAssets = InSelectedAssets.Num() > 0 ? InSelectedAssets : UEditorUtilityLibrary::GetSelectedAssetData();
	if (SelectedAssets.Num() == 0)
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Assets Selected"));
		return;
	}
	uint32 Counter = 0;
	NumOdDuplicates = FMath::Clamp(NumOdDuplicates,2,10);
	for (const FAssetData& AssetData : SelectedAssets)
	{
		for (int i = 0; i < NumOdDuplicates; i++)
		{
			// get the full path in the editor .example ( /game/folder/asset )
			const FString SourceAssetPath = AssetData.GetObjectPathString();
			
			// create a new name based on the selected asset and add some strings 
			const FString NewSelectedAssetName = AssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i+1);
			
			// will return the folder name of the asset 
			const FString NewPathName = FPaths::Combine(AssetData.PackagePath.ToString(),NewSelectedAssetName);
			
			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath,NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName,false);
				++Counter;
			}
		}
	}
	
	if (Counter >0)
	{
		//DebugHelper::PrintDebugScreen(TEXT("Successfully Duplicated " + FString::FromInt(Counter) + "Files"),FColor::MakeRandomColor(),7.0f);
		DebugHelper::ShowNotifyInfo(TEXT("Successfully Duplicated " + FString::FromInt(Counter) + " Assets"));
	}
	
}

void UAutomatedAssetActions::ApplyNamingConvention(TArray<UObject*> InSelectedAssets)
{
	const UEngineMateSettings* Settings = GetDefault<UEngineMateSettings>();
	TArray<UObject*> SelectedAssets = InSelectedAssets.Num() > 0 ? InSelectedAssets : UEditorUtilityLibrary::GetSelectedAssets();
	if (SelectedAssets.Num() == 0)
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Assets Selected"));
		return;
	}

	uint32 Counter = 0;

	for (UObject* Asset : SelectedAssets)
	{
		if (!Asset) continue;

		FString PrefixFound;

		// -----------------------------
		// 1. Handle Blueprint Assets
		// -----------------------------
		if (const UBlueprint* BP = Cast<UBlueprint>(Asset))
		{
			if (BP->GeneratedClass)
			{
				// Widget Blueprint → WBP_
				if (BP->GeneratedClass->IsChildOf(UUserWidget::StaticClass()))
				{
					PrefixFound = TEXT("WBP_");
				}
				// (Optional) Add more Blueprint type rules here later
			}
		}

		// -----------------------------
		// 2. Fallback to Map Lookup
		// -----------------------------
		if (PrefixFound.IsEmpty())
		{
			// Exact match first
			if (const FString* Found = Settings->PreFixNamingMap.Find(Asset->GetClass()))
			{
				PrefixFound = *Found;
			}
			else
			{
				// Inheritance-based fallback (best match)
				UClass* BestMatchClass = nullptr;

				for (const auto& Pair : Settings->PreFixNamingMap)
				{
					if (Asset->IsA(Pair.Key))
					{
						if (!BestMatchClass || Asset->GetClass()->IsChildOf(BestMatchClass))
						{
							BestMatchClass = Pair.Key;
							PrefixFound = Pair.Value;
						}
					}
				}
			}
		}

		// -----------------------------
		// 3. Validation
		// -----------------------------
		if (PrefixFound.IsEmpty())
		{
			DebugHelper::PrintDebugScreen(
				TEXT("Failed To Find Prefix for class ") + Asset->GetClass()->GetName(),
				FColor::Red,
				6.0f
			);
			continue;
		}

		FString OldName = Asset->GetName();

		// Already has prefix
		if (OldName.StartsWith(PrefixFound))
		{
			DebugHelper::PrintDebugScreen(
				OldName + TEXT(" Already Has Prefix Added "),
				FColor::Yellow,
				4.0f
			);
			continue;
		}

		// -----------------------------
		// 4. Special Cases
		// -----------------------------
		if (Asset->IsA(UMaterialInstanceConstant::StaticClass()))
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		// -----------------------------
		// 5. Rename
		// -----------------------------
		const FString NewNameWithPrefix = PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(Asset, NewNameWithPrefix);

		++Counter;
	}

	// -----------------------------
	// 6. Result Notification
	// -----------------------------
	if (Counter > 0)
	{
		DebugHelper::ShowNotifyInfo(
			TEXT("Successfully Renamed ") + FString::FromInt(Counter) + TEXT(" Assets")
		);
	}
}
void UAutomatedAssetActions::DeleteUnusedAssets(TArray<FAssetData> InSelectedAssets)
{
	TArray<FAssetData> SelectedAssets = InSelectedAssets.Num() > 0 ? InSelectedAssets : UEditorUtilityLibrary::GetSelectedAssetData();
	if (SelectedAssets.Num() == 0)
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Assets Selected"));
		return;
	}
	TArray<FAssetData> UnusedAssets;
	FixUpReDirectors();
	for (const FAssetData& AssetData : SelectedAssets)
	{
		TArray<FString> AssetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetData.GetObjectPathString());
		if (AssetReferences.IsEmpty())
		{
			UnusedAssets.Add(AssetData);
		}
	}
	if (UnusedAssets.IsEmpty())
	{
		DebugHelper::ShowMessageDialog(EAppMsgType::Type::Ok,TEXT("No Unused Asset Found Among Selected Assets"),false);
		return;
	}
	
	uint32 AssetsDeleted = ObjectTools::DeleteAssets(UnusedAssets);
	if (AssetsDeleted == 0) return;
	DebugHelper::ShowNotifyInfo(TEXT("Successfully Deleted ")+ FString::FromInt(AssetsDeleted)+TEXT(" Unused Assets"));
}
