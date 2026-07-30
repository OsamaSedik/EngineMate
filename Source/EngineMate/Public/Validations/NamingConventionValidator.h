// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "NamingConventionValidator.generated.h"


UCLASS()
class ENGINEMATE_API UNamingConventionValidator : public UEditorValidatorBase
{
	GENERATED_BODY()
	
protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
	

	
};
