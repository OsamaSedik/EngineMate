// Fill out your copyright notice in the Description page of Project Settings.


#pragma once
#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "AutomatedAssetActions.generated.h"


UCLASS()
class ENGINEMATE_API UAutomatedAssetActions : public UAssetActionUtility
{
	GENERATED_BODY()
	
	void FixUpReDirectors();
	
public:
	UFUNCTION(CallInEditor)
	void DuplicateAsset(int32 NumOdDuplicates);
	
	UFUNCTION(CallInEditor)
	void ApplyNamingConvention();
	
	UFUNCTION(CallInEditor)
	void DeleteUnusedAssets();
	
};
