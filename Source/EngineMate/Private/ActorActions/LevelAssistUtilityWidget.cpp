// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorActions/LevelAssistUtilityWidget.h"
#include "CineCameraActor.h"
#include "Components/LightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DecalActor.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/Light.h"
#include "Engine/SkyLight.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/DebugHeader.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "NiagaraActor.h"
#include "GameFramework/PlayerStart.h"
#include "Particles/Emitter.h"


bool ULevelAssistUtilityWidget::GetEditorActorSubsystem()
{
	if (!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return EditorActorSubsystem != nullptr;
}


 int32 ULevelAssistUtilityWidget::GetResolutionValue(const ETextureResolution Resolution)
{
	switch (Resolution)
	{
	case ETextureResolution::R_256:  return 256;
	case ETextureResolution::R_512:  return 512;
	case ETextureResolution::R_1024: return 1024;
	case ETextureResolution::R_2048: return 2048;
	case ETextureResolution::R_4096: return 4096;
	case ETextureResolution::R_8192: return 8192;
	default: return 0;
	}
}

void ULevelAssistUtilityWidget::SelectAllActorsWithSimilarName()
{
	if (!GetEditorActorSubsystem()) return;
	const TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 SelectionCounter = 0;
	if (SelectedActors.IsEmpty())
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Actors Has Been Selected."));
		return;
	}
	if (SelectedActors.Num() > 1)
	{
		DebugHelper::ShowNotifyInfo(TEXT("You Can Select Only One Actor."));
		return;
	}
	const FString SelectedActorName = SelectedActors[0]->GetActorLabel();
	const FString NameToSearch = SelectedActorName.LeftChop(4);
	
	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();
	for (AActor* Actor : AllLevelActors)
	{
		if (!Actor) continue;
		if (Actor->GetActorLabel().Contains(NameToSearch,SearchCase))
		{
			EditorActorSubsystem->SetActorSelectionState(Actor,true);
			++SelectionCounter;
		}
	}
	if (SelectionCounter > 0)
	{
		DebugHelper::ShowNotifyInfo(TEXT("Successfully selected ")+FString::FromInt(SelectionCounter)+ TEXT(" Actors In Level "));
	}
	else
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Actors With Similar Name Found."));
	}
}

void ULevelAssistUtilityWidget::OrganizeOutlinerByType()
{
	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetAllLevelActors();

	if (SelectedActors.Num() == 0)
	{
		DebugHelper::ShowNotifyInfo(TEXT("No actors selected."));
		return;
	}

	uint32 Counter = 0;

	for (AActor* Actor : SelectedActors)
	{
		if (!Actor) continue;

		Actor->Modify();

		const FString CurrentFolder = Actor->GetFolderPath().ToString();
		FString TargetFolder;

		// =========================================================
		// LIGHTS + SKY + ATMOSPHERE
		// =========================================================
		if (Actor->IsA<ALight>() ||
			Actor->IsA<ASkyLight>() ||
			Actor->IsA<AExponentialHeightFog>() ||
			Actor->IsA<ASkyAtmosphere>() ||
			Actor->IsA<AVolumetricCloud>())
		{
			TargetFolder = TEXT("Lights");
		}

		// =========================================================
		// BLUEPRINTS
		// =========================================================
		else if (Actor->GetClass()->ClassGeneratedBy != nullptr)
		{
			TargetFolder = TEXT("Blueprints");
		}

		// =========================================================
		// EFFECTS (NIAGARA + CASCADE)
		// =========================================================
		else if (Actor->IsA<ANiagaraActor>() ||
				 Actor->IsA<AEmitter>() ||
				 Actor->GetClass()->GetName().Contains(TEXT("Niagara")))
		{
			TargetFolder = TEXT("Effects");
		}

		// =========================================================
		// DECALS
		// =========================================================
		else if (Actor->IsA<ADecalActor>())
		{
			TargetFolder = TEXT("Decals");
		}

		// =========================================================
		// STATIC MESH
		// =========================================================
		else if (Actor->IsA<AStaticMeshActor>())
		{
			TargetFolder = TEXT("Static Mesh");
		}

		// =========================================================
		// SKELETAL MESH
		// =========================================================
		else if (Actor->FindComponentByClass<USkeletalMeshComponent>())
		{
			TargetFolder = TEXT("Skeletal Mesh");
		}

		// =========================================================
		// PLAYER START
		// =========================================================
		else if (Actor->IsA<APlayerStart>())
		{
			TargetFolder = TEXT("Player Start");
		}

		// =========================================================
		// POST PROCESS
		// =========================================================
		else if (Actor->IsA<APostProcessVolume>())
		{
			TargetFolder = TEXT("Post Process");
		}

		// =========================================================
		// VOLUMES
		// =========================================================
		else if (Actor->IsA<AVolume>())
		{
			TargetFolder = TEXT("Volumes");
		}

		// =========================================================
		// CINEMATIC
		// =========================================================
		else if (Actor->IsA<ACineCameraActor>() ||
				 Actor->IsA<ACameraActor>() ||
				 Actor->GetClass()->GetName().Contains(TEXT("Cine")))
		{
			TargetFolder = TEXT("Cinematic");
		}

		// =========================================================
		// APPLY ONLY IF DIFFERENT
		// =========================================================
		if (!TargetFolder.IsEmpty() && CurrentFolder != TargetFolder)
		{
			Actor->SetFolderPath(FName(*TargetFolder));
			++Counter;
		}
	}

	DebugHelper::ShowNotifyInfo(
		FString::Printf(TEXT("Organized %d actors into folders"), Counter)
	);
}


void ULevelAssistUtilityWidget::SetLightChannelsForSelectedActors()
{
	UKismetSystemLibrary::BeginTransaction(TEXT("LevelAssist.ActorsLightChannels"),FText::FromString(TEXT("Set Light Channels For Mesh")),
		GetWorld());
	uint32 Counter = 0;
	if (!GetEditorActorSubsystem()) return;
	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	if (SelectedActors.IsEmpty())
	{
		DebugHelper::ShowMessageDialog(EAppMsgType::Ok,TEXT("Please Select an Actor."),true);
		return;
	}
	for (AActor* Actor : SelectedActors)
	{
		if (!Actor) continue;

		TArray<UStaticMeshComponent*> MeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
		
		if (MeshComponents.Num()>0)
		{
			UKismetSystemLibrary::TransactObject(Actor);
			for (UStaticMeshComponent* MeshComp : MeshComponents)
			{
				if (!MeshComp) continue;
				MeshComp->Modify();
				MeshComp->SetLightingChannels(LightChannels.LightChannelOne,LightChannels.LightChannelTwo,LightChannels.LightChannelThree);
				MeshComp->MarkRenderStateDirty();
				++Counter;
			}
		}
	}
	if (Counter > 0)
	{
		DebugHelper::ShowNotifyInfo(
			FString::Printf(TEXT("Successfully Changed Light Channels on %d Actors"), Counter)
			
		);
	}
	else
	{
		DebugHelper::ShowNotifyInfo(TEXT("No static mesh components found."));
	}
	
	UKismetSystemLibrary::EndTransaction();
}

void ULevelAssistUtilityWidget::SetLightChannelsForSelectedLights()
{
	UKismetSystemLibrary::BeginTransaction(TEXT("LevelAssist.LightsLightChannels"),FText::FromString(TEXT("Set Light Channels For Lights")),
	GetWorld());
	uint32 Counter = 0;
	if (!GetEditorActorSubsystem()) return;
	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	if (SelectedActors.IsEmpty())
	{
		FString MSG = FString::Printf(TEXT("No Lights Has Been Selected \n Please Select One."));
		DebugHelper::ShowMessageDialog(EAppMsgType::Ok,MSG,true);
		return;
	}
	for (AActor* Actor : SelectedActors)
	{
		if (!Actor) continue;
		if (!Actor->IsA(ALight::StaticClass()))
		{
			DebugHelper::ShowMessageDialog(EAppMsgType::Ok, TEXT("Please Select Only Lights"),true);
			return;;
		}
		
		// 1. Handle ALight actors
		if (ALight* Light = Cast<ALight>(Actor))
		{
			UKismetSystemLibrary::TransactObject(Light);
			if (ULightComponent* LightComp = Light->GetLightComponent())
			{
				LightComp->Modify();
				LightComp->SetLightingChannels(LightChannels.LightChannelOne,LightChannels.LightChannelTwo,LightChannels.LightChannelThree);
				LightComp->MarkRenderStateDirty();
				++Counter;
			}
			continue;
		}
		// 2. Handle actors with light components
		TArray<ULightComponent*> LightComponents;
		Actor->GetComponents<ULightComponent>(LightComponents);
		if (LightComponents.Num()>0)
		{
			UKismetSystemLibrary::TransactObject(Actor);
			for (ULightComponent* Comp : LightComponents)
			{
				if (!Comp) continue;
				Comp->Modify();
				Comp->SetLightingChannels(LightChannels.LightChannelOne,LightChannels.LightChannelTwo,LightChannels.LightChannelThree);
				Comp->MarkRenderStateDirty();
				++Counter;
			}
		}
	}
	if (Counter > 0)
	{
		DebugHelper::ShowNotifyInfo(
			FString::Printf(TEXT("Successfully Changed Light Channels on %d Selected Lights"), Counter)
			
		);
	}
	UKismetSystemLibrary::EndTransaction();
}

void ULevelAssistUtilityWidget::SelectAllActorsByClass()
{
	if (!GetEditorActorSubsystem()) return;
	EditorActorSubsystem->ClearActorSelectionSet();
	if (SelectedClass == nullptr)
	{
		DebugHelper::ShowMessageDialog(EAppMsgType::Ok, TEXT("Please Select a Class Type "),true);
		return;
	}
	const TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();
	uint32 SelectionCounter = 0;
	if (AllLevelActors.IsEmpty())
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Actors Has Been Selected Within Your Class Type Selection."));
		return;
	}
	for (AActor* LevelActor : AllLevelActors)
	{
		if (LevelActor->IsA(SelectedClass))
		{
			EditorActorSubsystem->SetActorSelectionState(LevelActor,true);
			++SelectionCounter;
		}
	}
	if (SelectionCounter > 0)
	{
		DebugHelper::ShowNotifyInfo(TEXT("Successfully Selected ")+ FString::FromInt(SelectionCounter) + TEXT(" Actors In Level "));
	}
}

void ULevelAssistUtilityWidget::SelectAllLevelLights()
{
	if (!GetEditorActorSubsystem()) return;
	const TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();
	EditorActorSubsystem->ClearActorSelectionSet();
	uint32 SelectionCounter = 0;
	if (AllLevelActors.IsEmpty())
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Lights Has Been Selected."));
		return;
	}
	for (AActor* LevelActor : AllLevelActors)
	{
		if (LevelActor->IsA(ALight::StaticClass()))
		{
			EditorActorSubsystem->SetActorSelectionState(LevelActor,true);
			++SelectionCounter;
		}
	}
	if (SelectionCounter > 0)
	{
		DebugHelper::ShowNotifyInfo(TEXT("Successfully Selected ")+ FString::FromInt(SelectionCounter) + TEXT(" Lights In Level "));
	}
}

void ULevelAssistUtilityWidget::DisableCastShadowsForLights()
{
	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.Num() == 0)
	{
		DebugHelper::ShowMessageDialog(
			EAppMsgType::Ok,
			TEXT("No Lights Selected. Please select at least one Light."),
			true
		);
		return;
	}

	// =========================================================
	// UNDO TRANSACTION
	// =========================================================
	FScopedTransaction Transaction(
		FText::FromString(TEXT("Disable Cast Shadows For Lights"))
	);

	uint32 Counter = 0;

	for (AActor* Actor : SelectedActors)
	{
		if (!Actor) continue;

		if (!Actor->IsA<ALight>())
		{
			continue;
		}

		Actor->Modify();

		if (ALight* Light = Cast<ALight>(Actor))
		{
			if (ULightComponent* LightComp = Light->GetLightComponent())
			{
				// IMPORTANT: modify component BEFORE change (undo safety)
				LightComp->Modify();

				LightComp->SetCastShadows(false);
				LightComp->MarkRenderStateDirty();

				++Counter;
			}
		}
	}

	if (Counter > 0)
	{
		DebugHelper::ShowNotifyInfo(
			FString::Printf(TEXT("Disabled Cast Shadows on %d Lights"), Counter)
		);
	}
	else
	{
		DebugHelper::ShowNotifyInfo(TEXT("No valid Lights were modified."));
	}
}


void ULevelAssistUtilityWidget::DisableCastShadowsForActors()
{
	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.Num() == 0)
	{
		DebugHelper::ShowMessageDialog(
			EAppMsgType::Ok,
			TEXT("No Actors Selected. Please select at least one Actor."),
			true
		);
		return;
	}

	FScopedTransaction Transaction(
		FText::FromString(TEXT("Disable Cast Shadows For Static Meshes"))
	);

	uint32 ComponentCounter = 0;

	for (AActor* Actor : SelectedActors)
	{
		if (!Actor) continue;

		TArray<UStaticMeshComponent*> MeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

		if (MeshComponents.Num() == 0)
		{
			continue;
		}

		Actor->Modify();

		for (UStaticMeshComponent* MeshComp : MeshComponents)
		{
			if (!MeshComp) continue;

			MeshComp->Modify();

			MeshComp->SetCastShadow(false);
			MeshComp->MarkRenderStateDirty();

			++ComponentCounter;
		}
	}

	if (ComponentCounter > 0)
	{
		DebugHelper::ShowNotifyInfo(
			FString::Printf(TEXT("Disabled Cast Shadows on %d Static Mesh Components"), ComponentCounter)
		);
	}
	else
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Static Mesh Components Found in Selection."));
	}
}

void ULevelAssistUtilityWidget::DisableMeshDistanceField()
{
	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.Num() == 0)
	{
		DebugHelper::ShowMessageDialog(
			EAppMsgType::Ok,
			TEXT("No Actors Selected. Please select at least one Actor."),
			true
		);
		return;
	}
	
	FScopedTransaction Transaction(
		FText::FromString(TEXT("Disable Mesh Distance Field"))
	);

	uint32 ComponentCounter = 0;

	for (AActor* Actor : SelectedActors)
	{
		if (!Actor) continue;

		TArray<UStaticMeshComponent*> MeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

		if (MeshComponents.Num() == 0)
		{
			continue;
		}

		// IMPORTANT: mark actor BEFORE changes for undo
		Actor->Modify();

		for (UStaticMeshComponent* MeshComp : MeshComponents)
		{
			if (!MeshComp) continue;

			// IMPORTANT: mark component BEFORE changes
			MeshComp->Modify();

			MeshComp->SetAffectDistanceFieldLighting(false);
			MeshComp->MarkRenderStateDirty();

			++ComponentCounter;
		}
	}

	if (ComponentCounter > 0)
	{
		DebugHelper::ShowNotifyInfo(
			FString::Printf(TEXT("Disabled Mesh Distance Field on %d Components"), ComponentCounter)
		);
	}
	else
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Static Mesh Components Found."));
	}
}


void ULevelAssistUtilityWidget::DisableCollisionOnActors()
{
	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.Num() == 0)
	{
		DebugHelper::ShowMessageDialog(
			EAppMsgType::Ok,
			TEXT("No Actors Selected.\nPlease Select Actor"),
			true
		);
		return;
	}
	
	FScopedTransaction Transaction(
		FText::FromString(TEXT("Disable Collision On Actors"))
	);

	uint32 ComponentCounter = 0;

	for (AActor* Actor : SelectedActors)
	{
		if (!Actor) continue;

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

		if (PrimitiveComponents.Num() == 0)
		{
			continue;
		}

		// Track actor for undo
		Actor->Modify();

		for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
		{
			if (!PrimComp) continue;

			// Only change if needed (prevents dirty spam + better undo)
			if (PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
			{
				// Track component for undo
				PrimComp->Modify();
				PrimComp->SetCollisionProfileName(TEXT("NoCollision"));
				PrimComp->SetGenerateOverlapEvents(false);

				// Optional: usually not required for collision, but safe
				PrimComp->MarkRenderStateDirty();

				++ComponentCounter;
			}
		}
	}

	if (ComponentCounter > 0)
	{
		DebugHelper::ShowNotifyInfo(
			FString::Printf(TEXT("Disabled collision on %d component(s)"), ComponentCounter)
		);
	}
	else
	{
		DebugHelper::ShowNotifyInfo(TEXT("No collision-enabled components found."));
	}
}


void ULevelAssistUtilityWidget::DisableTickOnSelectedActors()
{
	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.Num() == 0)
	{
		DebugHelper::ShowMessageDialog(
			EAppMsgType::Ok,
			TEXT("No Blueprint Actors Selected."),
			true
		);
		return;
	}

	FScopedTransaction Transaction(
		FText::FromString(TEXT("Disable Tick On Blueprint Actors"))
	);

	uint32 Counter = 0;

	for (AActor* Actor : SelectedActors)
	{
		if (!Actor || !Actor->GetClass()) continue;

		// =========================================================
		// ONLY PROCESS BLUEPRINT ACTORS
		// =========================================================
		const bool bIsBlueprint = (Actor->GetClass()->ClassGeneratedBy != nullptr);

		if (!bIsBlueprint)
		{
			continue;
		}

		Actor->Modify();

		// =========================================================
		// 1. Disable ACTOR tick
		// =========================================================
		Actor->PrimaryActorTick.bCanEverTick = false;
		Actor->SetActorTickEnabled(false);
		Actor->PrimaryActorTick.SetTickFunctionEnable(false);

		// =========================================================
		// 2. Disable ALL component ticks
		// =========================================================
		TArray<UActorComponent*> Components = Actor->GetComponents().Array();

		for (UActorComponent* Comp : Components)
		{
			if (!Comp) continue;

			Comp->Modify();
			Comp->PrimaryComponentTick.bCanEverTick = false;
			Comp->PrimaryComponentTick.SetTickFunctionEnable(false);
		}

		// =========================================================
		// 3. Prevent tick interval updates
		// =========================================================
		Actor->SetActorTickInterval(0.f);

		++Counter;
	}

	if (Counter == 0)
	{
		DebugHelper::ShowMessageDialog(
			EAppMsgType::Ok,
			TEXT("No Blueprint Actors Found in Selection."),
			true
		);
		return;
	}

	DebugHelper::ShowNotifyInfo(
		FString::Printf(TEXT("Disabled Tick on %d Blueprint Actors"), Counter)
	);
}



void ULevelAssistUtilityWidget::DeleteNullActors()
{
	UKismetSystemLibrary::BeginTransaction(TEXT("LevelAssist.NullActors"),
		FText::FromString(TEXT("Delete Null Actors")), GetWorld());
	uint32 Counter = 0;
	if (!GetEditorActorSubsystem()) return;
	EditorActorSubsystem->ClearActorSelectionSet();
	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();

	if (AllLevelActors.IsEmpty())
	{
		DebugHelper::ShowNotifyInfo(TEXT("No actors selected."));
		return;
	}
	for (AActor* Actor : AllLevelActors)
	{
		if (!Actor) continue;
		if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
		{
			UKismetSystemLibrary::TransactObject(MeshActor);
			if (!MeshActor->GetStaticMeshComponent()->GetStaticMesh())
			{
				EditorActorSubsystem->DestroyActor(MeshActor);
				++Counter;
			}
		}
	}
	if (Counter > 0)
	{
		DebugHelper::ShowNotifyInfo(TEXT("Successfully Delete ")+ FString::FromInt(Counter)+ TEXT(" Null Static Meshes In Level "));
	}
	UKismetSystemLibrary::EndTransaction();
}

void ULevelAssistUtilityWidget::SelectActorsByTextureResolution()
{
	if (!GetEditorActorSubsystem()) return;

	const int32 TargetSize = GetResolutionValue(DesireResolution);
	EditorActorSubsystem->ClearActorSelectionSet();
	TArray<AActor*> AllActors = EditorActorSubsystem->GetAllLevelActors();

	if (AllActors.IsEmpty())
	{
		DebugHelper::ShowNotifyInfo(TEXT("No actors in level."));
		return;
	}

	EditorActorSubsystem->ClearActorSelectionSet();

	uint32 Counter = 0;

	for (AActor* Actor : AllActors)
	{
		if (!Actor) continue;

		TArray<UStaticMeshComponent*> MeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

		bool bMatchFound = false;

		for (UStaticMeshComponent* MeshComp : MeshComponents)
		{
			if (!MeshComp) continue;

			const int32 MaterialCount = MeshComp->GetNumMaterials();

			for (int32 i = 0; i < MaterialCount; i++)
			{
				UMaterialInterface* Material = MeshComp->GetMaterial(i);
				if (!Material) continue;

				TArray<UTexture*> Textures;
				Material->GetUsedTextures(
					Textures,
					EMaterialQualityLevel::High,
					true,
					ERHIFeatureLevel::SM5,
					true
				);

				for (UTexture* Tex : Textures)
				{
					UTexture2D* Tex2D = Cast<UTexture2D>(Tex);
					if (!Tex2D) continue;

					const int32 MaxRes = FMath::Max(Tex2D->GetSizeX(), Tex2D->GetSizeY());

					if (MaxRes == TargetSize)
					{
						bMatchFound = true;
						break;
					}
				}

				if (bMatchFound) break;
			}

			if (bMatchFound) break;
		}

		if (bMatchFound)
		{
			EditorActorSubsystem->SetActorSelectionState(Actor, true);
			++Counter;
		}
	}

	if (Counter > 0)
	{
		DebugHelper::ShowNotifyInfo(
		FString::Printf(TEXT("Selected %d actors using %d texture resolution"),
			Counter, TargetSize)
	);
	}
	else
	{
		DebugHelper::ShowMessageDialog(EAppMsgType::Ok,TEXT("No actors Found By the Selected Resolution."),false);
	}
	
}

void ULevelAssistUtilityWidget::DuplicateActors()
{
	if (!GetEditorActorSubsystem()) return;
	const TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 SelectionCounter = 0;
	if (SelectedActors.IsEmpty())
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Actors Has Been Selected."));
		return;
	}
	if (NumberOfDuplicates <= 0|| OffsetDistance == 0.0f)
	{
		DebugHelper::ShowNotifyInfo(TEXT("Didn't Specify Number For Duplication Or Offset "));
		return;
	}
	UKismetSystemLibrary::BeginTransaction(TEXT("LevelAssist.Duplication"),FText::FromString(TEXT("Duplicate Actors")), GetWorld());
	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor) continue;
		for (int i = 0 ; i < NumberOfDuplicates ; ++i)
		{
			AActor* DuplicatedActor = EditorActorSubsystem->DuplicateActor(SelectedActor,SelectedActor->GetWorld());
			UKismetSystemLibrary::TransactObject(DuplicatedActor);
			if (!DuplicatedActor) continue;
			const float DuplicationOffsetDist = (i+1) * OffsetDistance;
			switch (DuplicatedAxis)
			{
			case EDuplicationAxis::EDA_XAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(DuplicationOffsetDist,0.0f,0.0f));
				break;
				
			case EDuplicationAxis::EDA_YAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(0.0f,DuplicationOffsetDist,0.0f));
				break;
				
			case EDuplicationAxis::EDA_ZAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(0.0f,0.0f,DuplicationOffsetDist));
				break;
			case EDuplicationAxis::EDA_Max:
				break;
			}
			EditorActorSubsystem->SetActorSelectionState(DuplicatedActor,true);
			++SelectionCounter;
		}
	}
	UKismetSystemLibrary::EndTransaction();
	if (SelectionCounter > 0)
	{
		DebugHelper::ShowNotifyInfo(TEXT("Successfully Duplicate ")+ FString::FromInt(SelectionCounter)+ TEXT(" Actors In Level "));
	}
}

void ULevelAssistUtilityWidget::RandomizeActorsTransform()
{
	const bool ConditionNotSet = RandomActorTransformation.bRandomizeYawRotation || 
		RandomActorTransformation.bRandomizePitchRotation || 
			RandomActorTransformation.bRandomizeRollRotation || RandomActorTransformation.bRandomizeOffset;
	if (!ConditionNotSet)
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Random Transformation Condition Set !!."));
	}
	if (!GetEditorActorSubsystem()) return;
	const TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 Counter = 0;
	if (SelectedActors.IsEmpty())
	{
		DebugHelper::ShowNotifyInfo(TEXT("No Actors Has Been Selected."));
		return;
	}
	UKismetSystemLibrary::BeginTransaction(TEXT("LevelAssist.Duplication"),FText::FromString(TEXT("Transform Actors Rotation")),
		GetWorld());
	
	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor) continue;
		UKismetSystemLibrary::TransactObject(SelectedActor);
		if (RandomActorTransformation.bRandomizeYawRotation)
		{
			const float RandomYaw = FMath::RandRange(RandomActorTransformation.RotYawMin,RandomActorTransformation.RotYawMax);
			SelectedActor->AddActorWorldRotation(FRotator(0.0f,RandomYaw,0.0f));
		}
		if (RandomActorTransformation.bRandomizePitchRotation)
		{
			const float RandomPitch = FMath::RandRange(RandomActorTransformation.RotYawMin,RandomActorTransformation.RotYawMax);
			SelectedActor->AddActorWorldRotation(FRotator(RandomPitch,0.0f,0.0f));
		}
		if (RandomActorTransformation.bRandomizeRollRotation)
		{
			const float RandomRoll = FMath::RandRange(RandomActorTransformation.RotYawMin,RandomActorTransformation.RotYawMax);
			SelectedActor->AddActorWorldRotation(FRotator(0.0,0.0f,RandomRoll));
		}
		if (RandomActorTransformation.bRandomizeOffset)
		{
			const float RandomOffset = FMath::RandRange(RandomActorTransformation.OffsetMin,RandomActorTransformation.OffsetMax);
			SelectedActor->AddActorWorldOffset(FVector(RandomOffset,RandomOffset,0.0f));
		}
		
		if (RandomActorTransformation.bRandomizeYawRotation || RandomActorTransformation.bRandomizePitchRotation || RandomActorTransformation.bRandomizeRollRotation)
		{
			++Counter;
		}
	}
	UKismetSystemLibrary::EndTransaction();
}


