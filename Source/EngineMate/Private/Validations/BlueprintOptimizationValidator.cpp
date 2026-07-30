// Fill out your copyright notice in the Description page of Project Settings.


#include "Validations/BlueprintOptimizationValidator.h"
#include "GameFramework/Character.h"
#include "Misc/DataValidation.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Settings/EngineMateSettings.h"

bool UBlueprintOptimizationValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject,
                                                                      FDataValidationContext& InContext) const
{
	const UEngineMateSettings* Settings = GetDefault<UEngineMateSettings>();
	return InObject && InObject->IsA(UBlueprint::StaticClass()) && Settings->bWarnOnTickUsage;
}




EDataValidationResult UBlueprintOptimizationValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData,
		UObject* InAsset,FDataValidationContext& Context)

{
	const UEngineMateSettings* EngineMateSettings = GetDefault<UEngineMateSettings>();
	// Resolve asset safely
	UObject* Asset = InAsset ? InAsset : InAssetData.GetAsset();

	UBlueprint* BP = Cast<UBlueprint>(Asset);

	// fallback for generated class cases
	if (!BP)
	{
		if (UBlueprintGeneratedClass* GenClass = Cast<UBlueprintGeneratedClass>(Asset))
		{
			BP = Cast<UBlueprint>(GenClass->ClassGeneratedBy);
		}
	}

	if (!BP)
	{
		return EDataValidationResult::NotValidated;
	}

	// ------------------------------------------------------------
	// 1. Detect Event Tick usage in graph
	// ------------------------------------------------------------
	bool bHasEventTick = false;

	for (UEdGraph* Graph : BP->UbergraphPages)
	{
		if (!Graph) continue;

		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (!Node) continue;

			const FString NodeTitle =
				Node->GetNodeTitle(ENodeTitleType::ListView).ToString();

			if (NodeTitle.Contains(TEXT("Event Tick")))
			{
				bHasEventTick = true;
				break;
			}
		}

		if (bHasEventTick)
		{
			break;
		}
	}

	// ------------------------------------------------------------
	// 2. Detect actual runtime Tick enabled state
	// ------------------------------------------------------------
	bool bTickEnabled = false;

	if (BP->GeneratedClass)
	{
		if (const AActor* CDO = Cast<AActor>(BP->GeneratedClass->GetDefaultObject()))
		{
			bTickEnabled = CDO->PrimaryActorTick.bStartWithTickEnabled;
		}
	}

	// ------------------------------------------------------------
	// 3. If no Tick usage at all → OK
	// ------------------------------------------------------------
	if (!bHasEventTick && !bTickEnabled)
	{
		return EDataValidationResult::Valid;
	}

	// ------------------------------------------------------------
	// 4. Whitelist important gameplay classes
	// ------------------------------------------------------------
	if (BP->ParentClass)
	{
		if (BP->ParentClass->IsChildOf(APawn::StaticClass()) ||
			BP->ParentClass->IsChildOf(ACharacter::StaticClass()) ||
			BP->ParentClass->IsChildOf(APlayerController::StaticClass()))
		{
			return EDataValidationResult::Valid;
		}
	}

	if (EngineMateSettings->bWarnOnTickUsage)
	{
		Context.AddWarning(FText::FromString(
		FString::Printf(
			TEXT("Blueprint '%s' has Tick enabled. Consider disabling Tick or using event-driven logic for better performance."),
			*BP->GetName()
		)
	));
	}
	
	return EDataValidationResult::Valid;
}