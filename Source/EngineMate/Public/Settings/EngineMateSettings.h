// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "LevelSequence.h"
#include "NiagaraSystem.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/Font.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Exporters/TextureCubeExporterHDR.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystem.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Retargeter/IKRetargeter.h"
#include "Sound/SoundCue.h"
#include "EngineMateSettings.generated.h"

UENUM()
enum class ETextureSize :uint8
{
	ETS_2K      UMETA(DisplayName = "2K"),
	ETS_4K      UMETA(DisplayName = "4K"),
	ETS_8K      UMETA(DisplayName = "8K"),
};

UCLASS(Config=Engine, DefaultConfig, meta=(DisplayName="Engine Mate Settings"))
class ENGINEMATE_API UEngineMateSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
protected:
	virtual FName GetCategoryName() const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	
	// Correct Prefix Naming Convention
	UPROPERTY(EditAnywhere, Config, Category="Automation|Assets")
	TMap<UClass*,FString> PreFixNamingMap = 
	{
		{UUserWidget::StaticClass(),TEXT("WBP_")},
		{UStaticMesh::StaticClass(),TEXT("SM_")},
		{USkeletalMesh::StaticClass(),TEXT("Sk_")},
		{UTexture2D::StaticClass(),TEXT("T_")},
		{UTextureCubeExporterHDR::StaticClass(),TEXT("HDR_")},
		{UMaterial::StaticClass(),TEXT("M_")},
		{UMaterialInstance::StaticClass(),TEXT("MI_")},
		{UMaterialInstanceConstant::StaticClass(),TEXT("MI_")},
		{UPhysicalMaterial::StaticClass(),TEXT("PM_")},
		{UActorComponent::StaticClass(),TEXT("AC_")},
		{UAnimationAsset::StaticClass(),TEXT("AS_")},
		{UAnimInstance::StaticClass(),TEXT("ABP_")},
		{UCurveTable::StaticClass(),TEXT("CT_")},
		{UDataTable::StaticClass(),TEXT("DT_")},
		{UEnum::StaticClass(),TEXT("E_")},
		{UParticleSystem::StaticClass(),TEXT("PS_")},
		{UNiagaraSystem::StaticClass(),TEXT("NS_")},
		{UAimOffsetBlendSpace::StaticClass(),TEXT("AO_")},
		{UAnimMontage::StaticClass(),TEXT("AM_")},
		{UBlendSpace::StaticClass(),TEXT("BS_")},
		{UBehaviorTree::StaticClass(),TEXT("BT_")},
		{UBlackboardData::StaticClass(),TEXT("BB_")},
		{UFont::StaticClass(),TEXT("Font_")},
		{USoundBase::StaticClass(),TEXT("S_")},
		{USoundCue::StaticClass(),TEXT("Cue_")},
		{USoundWave::StaticClass(),TEXT("Wav_")},
		{ULevelSequence::StaticClass(),TEXT("LS_")},
		{UIKRetargeter::StaticClass(),TEXT("RTG_")},
		{UIKRigDefinition::StaticClass(),TEXT("IK_")},
		{UInputAction::StaticClass(),TEXT("IA_")},
		{UInputMappingContext::StaticClass(),TEXT("IMC_")},
		{UPhysicsAsset::StaticClass(),TEXT("PHYS_")},
		{UTextureRenderTarget2D::StaticClass(),TEXT("RT_")},
	    {UBlueprint::StaticClass(),TEXT("BP_")},
        {UInterface::StaticClass(),TEXT("BPI_")},
		};
	
	
	// Texture Validation
	UPROPERTY(EditAnywhere, Config, Category="Validation|Textures")
	ETextureSize MaxTextureSize = ETextureSize::ETS_4K;
	
	UPROPERTY(EditAnywhere, Config, Category="Validation|Textures")
	bool bShowErrorOnValidation = false;
	
	// Blueprint Validation
	UPROPERTY(EditAnywhere, Config, Category="Validation|Blueprints")
	bool bWarnOnTickUsage = true;
	
	UPROPERTY(EditAnywhere, Config, Category="Validation|Blueprints")
	bool bWarnOnEmptyStaticMeshComponents = true;
	
	UPROPERTY(EditAnywhere, Config, Category="Validation|Blueprints")
    bool bWarnOnEmptySkeletalMeshComponents = true;
	
	// Static Mesh Validation
	UPROPERTY(EditAnywhere, Config, Category="Validation|Meshes")
	int32 MinLODCount = 2;

	UPROPERTY(EditAnywhere, Config, Category="Validation|Meshes")
	bool bRequireCollision = true;
};
