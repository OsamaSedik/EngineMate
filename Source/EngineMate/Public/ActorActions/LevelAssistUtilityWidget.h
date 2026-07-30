// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "LevelAssistUtilityWidget.generated.h"

UENUM(BlueprintType)
enum class EDuplicationAxis : uint8
{
	EDA_XAxis	UMETA(DisplayName="X Axis"),
	EDA_YAxis	UMETA(DisplayName="Y Axis"),
	EDA_ZAxis	UMETA(DisplayName="Z Axis"),
	EDA_Max		UMETA(DisplayName="Max"),
};

UENUM(BlueprintType)
enum class ETextureResolution : uint8
{
	R_256   UMETA(DisplayName = "256x256"),
	R_512   UMETA(DisplayName = "512x512"),
	R_1024  UMETA(DisplayName = "1024x1024"),
	R_2048  UMETA(DisplayName = "2048x2048"),
	R_4096  UMETA(DisplayName = "4096x4096"),
	R_8192  UMETA(DisplayName = "8192x8192")
};

USTRUCT(BlueprintType)
struct FLightChannel
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Assist")
	bool LightChannelOne = true;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Assist")
	bool LightChannelTwo = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Assist")
	bool LightChannelThree = false;
};
USTRUCT(BlueprintType)
struct FRandomActorTransformation
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Assist")
	bool bRandomizeYawRotation;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(EditCondition = bRandomizeYawRotation),Category = "LevelAssist")
	float RotYawMin = -45.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(EditCondition = bRandomizeYawRotation),Category = "LevelAssist")
	float RotYawMax = 45.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Assist")
	bool bRandomizePitchRotation;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(EditCondition = bRandomizePitchRotation),Category = "LevelAssist")
	float RotPitchMin = -45.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(EditCondition = bRandomizePitchRotation),Category = "LevelAssist")
	float RotPitchMax = 45.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Assist")
	bool bRandomizeRollRotation;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(EditCondition = bRandomizeRollRotation),Category = "LevelAssist")
	float RotRollMin = -45.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(EditCondition = bRandomizeRollRotation),Category = "LevelAssist")
	float RotRollMax = 45.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Assist")
	bool bRandomizeOffset = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(EditCondition = bRandomizeOffset),Category = "LevelAssist")
	float OffsetMin = -50.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(EditCondition = bRandomizeOffset),Category = "LevelAssist")
	float OffsetMax = 50.0f;
};


UCLASS()
class ENGINEMATE_API ULevelAssistUtilityWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
public:
#pragma region SELECTION
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Level Assist - Selction")
	TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::Type::IgnoreCase;
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void SelectAllActorsWithSimilarName();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Level Assist - Selction")
	TSubclassOf<AActor> SelectedClass;
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void SelectAllActorsByClass();
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void SelectAllLevelLights();
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	int32 GetResolutionValue(const ETextureResolution Resolution);

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Level Assist - Selction")
	ETextureResolution DesireResolution = ETextureResolution::R_2048;
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void SelectActorsByTextureResolution();
	
#pragma endregion

#pragma region OPTIMIZATION
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void DisableCastShadowsForLights();
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void DisableCastShadowsForActors();
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void DisableMeshDistanceField();
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void DisableCollisionOnActors();
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void DisableTickOnSelectedActors();
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void DeleteNullActors();

#pragma endregion
	
#pragma region MODIFY
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void OrganizeOutlinerByType();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Level Assist - Modify")
	FLightChannel LightChannels;
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void SetLightChannelsForSelectedActors();
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void SetLightChannelsForSelectedLights();
	
#pragma endregion
#pragma region DUPLICATION
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void DuplicateActors();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Level Assist - Duplication")
	int32 NumberOfDuplicates = 5;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Level Assist - Duplication")
	float OffsetDistance = 300.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Level Assist - Duplication")
	EDuplicationAxis DuplicatedAxis = EDuplicationAxis::EDA_XAxis;
	
#pragma endregion
	

#pragma region TRANSFORMATION
	
	UFUNCTION(BlueprintCallable, Category = "LevelAssist")
	void RandomizeActorsTransform();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Level Assist - Transform")
	FRandomActorTransformation RandomActorTransformation;
	
#pragma endregion
private:
	
	UPROPERTY()
	class UEditorActorSubsystem* EditorActorSubsystem;
	bool GetEditorActorSubsystem();
};
