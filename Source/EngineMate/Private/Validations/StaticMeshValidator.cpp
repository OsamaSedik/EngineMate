// Fill out your copyright notice in the Description page of Project Settings.


#include "Validations/StaticMeshValidator.h"
#include "Misc/DataValidation.h"
#include "PhysicsEngine/BodySetup.h"
#include "Settings/EngineMateSettings.h"

class UEngineMateSettings;

bool UStaticMeshValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject,
                                                           FDataValidationContext& InContext) const
{
	const UEngineMateSettings* Settings = GetDefault<UEngineMateSettings>();
	return InObject && InObject->IsA<UStaticMesh>() && Settings->bRequireCollision && Settings->MinLODCount >0;
}

EDataValidationResult UStaticMeshValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData,
	UObject* InAsset, FDataValidationContext& Context)
{
	const UEngineMateSettings* EngineMateSettings = GetDefault<UEngineMateSettings>();
	const UStaticMesh* Mesh = Cast<UStaticMesh>(InAsset);
	if (!Mesh)
	{
		return EDataValidationResult::NotValidated;
	}

	// ------------------------------------------------------------
	// LOD validation (correct method)
	// ------------------------------------------------------------
	if (!Mesh->GetRenderData())
	{
		Context.AddError(FText::FromString("Static mesh has no LOD"));
		return EDataValidationResult::Invalid;
	}

	const FStaticMeshLODResourcesArray& LODs = Mesh->GetRenderData()->LODResources;

	if (LODs.Num() < EngineMateSettings->MinLODCount)
	{
		Context.AddError(FText::Format(
		NSLOCTEXT("Validation", "LowLODCount",
			"Static Mesh has insufficient LODs. Required: {0}, Found: {1}"),
			FText::AsNumber(EngineMateSettings->MinLODCount),
			FText::AsNumber(LODs.Num())
			));
		return EDataValidationResult::Invalid;
	}

	// ------------------------------------------------------------
	// Collision validation (safe check)
	// ------------------------------------------------------------
	if (!Mesh->GetBodySetup() ||
		Mesh->GetBodySetup()->AggGeom.GetElementCount() == 0)
	{
		if (EngineMateSettings->bRequireCollision)
		{
			Context.AddError(FText::FromString("Static mesh has no valid collision"));
			return EDataValidationResult::Invalid;
		}
	}

	// ------------------------------------------------------------
	// Success
	// ------------------------------------------------------------
	Context.AddMessage(
		EMessageSeverity::Info,
		FText::FromString("Static Mesh validation passed")
	);

	return EDataValidationResult::Valid;
}
