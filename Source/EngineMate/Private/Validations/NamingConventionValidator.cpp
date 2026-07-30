// Fill out your copyright notice in the Description page of Project Settings.


#include "Validations/NamingConventionValidator.h"
#include "Misc/DataValidation.h"
#include "Settings/EngineMateSettings.h"

bool UNamingConventionValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) const
{
	const UEngineMateSettings* Settings = GetDefault<UEngineMateSettings>();
	// Return true if the asset class exists in your prefix map
	for (auto& Pair : Settings->PreFixNamingMap)
	{
		if (InAsset->IsA(Pair.Key)) return true;
	}
	return false;
}

EDataValidationResult UNamingConventionValidator::ValidateLoadedAsset_Implementation(
	const FAssetData& InAssetData,
	UObject* InAsset,
	FDataValidationContext& Context)
{
	const UEngineMateSettings* Settings = GetDefault<UEngineMateSettings>();
	const FString AssetName = InAsset->GetName();

	FString RequiredPrefix;

	// -----------------------------
	// 1. Handle Blueprint Assets
	// -----------------------------
	if (const UBlueprint* BP = Cast<UBlueprint>(InAsset))
	{
		if (BP->GeneratedClass)
		{
			// Widget Blueprint → WBP_
			if (BP->GeneratedClass->IsChildOf(UUserWidget::StaticClass()))
			{
				RequiredPrefix = TEXT("WBP_");
			}
		}
	}

	// -----------------------------
	// 2. Fallback to Map (Best Match)
	// -----------------------------
	if (RequiredPrefix.IsEmpty())
	{
		UClass* BestMatchClass = nullptr;

		for (const auto& Pair : Settings->PreFixNamingMap)
		{
			if (InAsset->IsA(Pair.Key))
			{
				if (!BestMatchClass || InAsset->GetClass()->IsChildOf(BestMatchClass))
				{
					BestMatchClass = Pair.Key;
					RequiredPrefix = Pair.Value;
				}
			}
		}
	}

	// -----------------------------
	// 3. Skip if no rule
	// -----------------------------
	if (RequiredPrefix.IsEmpty())
	{
		return EDataValidationResult::NotValidated;
	}

	// -----------------------------
	// 4. Validate
	// -----------------------------
	if (AssetName.StartsWith(RequiredPrefix))
	{
		AssetPasses(InAsset);

		Context.AddMessage(
			EMessageSeverity::Info,
			FText::FromString(TEXT("Asset validation passed"))
		);

		return EDataValidationResult::Valid;
	}

	// -----------------------------
	// 5. Fail
	// -----------------------------
	AssetFails(
		InAsset,
		FText::FromString(
			FString::Printf(
				TEXT("Asset '%s' must start with prefix '%s'"),
				*AssetName,
				*RequiredPrefix
			)
		)
	);

	return EDataValidationResult::Invalid;
}