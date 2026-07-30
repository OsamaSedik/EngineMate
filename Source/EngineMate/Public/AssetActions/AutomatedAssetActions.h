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
	void DuplicateAsset(int32 NumOdDuplicates, TArray<FAssetData> InSelectedAssets = TArray<FAssetData>());
	
	UFUNCTION(CallInEditor)
	void ApplyNamingConvention(TArray<UObject*> InSelectedAssets = TArray<UObject*>());
	
	UFUNCTION(CallInEditor)
	void DeleteUnusedAssets(TArray<FAssetData> InSelectedAssets = TArray<FAssetData>());
	
};
