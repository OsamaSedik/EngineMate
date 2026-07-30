// Fill out your copyright notice in the Description page of Project Settings.


#include "Validations/TextureSizeValidator.h"
#include "Misc/DataValidation.h"
#include "Settings/EngineMateSettings.h"


class UEngineMateSettings;

bool UTextureSizeValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject,
                                                            FDataValidationContext& InContext) const
{
	return InObject && InObject->IsA<UTexture>();
}

EDataValidationResult UTextureSizeValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData,
	UObject* InAsset, FDataValidationContext& Context)
{
	const UEngineMateSettings* Settings = GetDefault<UEngineMateSettings>();
	UTexture2D* Tex2D = Cast<UTexture2D>(InAsset);
	if (!Tex2D)
	{
		return EDataValidationResult::NotValidated;
	}

	// Get Limit from Settings
	int32 TextureSizeLimit = 0;
	switch (Settings->MaxTextureSize)
	{
	case ETextureSize::ETS_2K: TextureSizeLimit = 2048; break;
	case ETextureSize::ETS_4K: TextureSizeLimit = 4096; break;
	case ETextureSize::ETS_8K: TextureSizeLimit = 8192; break;
	}
	
	const int32 ImportedMaxSize = Tex2D->GetMaximumDimension();

	const int32 EffectiveMaxSize = Tex2D->MaxTextureSize > 0 ? Tex2D->MaxTextureSize : ImportedMaxSize;
	
	if (EffectiveMaxSize > TextureSizeLimit)
	{
		if (Settings->bShowErrorOnValidation)
		{
			Context.AddError(FText::FromString(
			FString::Printf(TEXT("Texture is too large. Limit: %d, Imported: %d, Effective: %d. Reduce size via MaxTextureSize or LODBias."),
				TextureSizeLimit, ImportedMaxSize, EffectiveMaxSize)));

			return EDataValidationResult::Invalid;
		}

		Context.AddWarning(FText::FromString(
			FString::Printf(TEXT("Texture is too large. Limit: %d, Imported: %d, Effective: %d. Reduce size via MaxTextureSize or LODBias."),
				TextureSizeLimit, ImportedMaxSize, EffectiveMaxSize)));

		return EDataValidationResult::Valid;
	}
	
	return EDataValidationResult::Valid;
}


