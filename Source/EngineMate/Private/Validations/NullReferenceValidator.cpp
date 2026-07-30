// Fill out your copyright notice in the Description page of Project Settings.


#include "Validations/NullReferenceValidator.h"

#include "Engine/SCS_Node.h"
#include "Misc/DataValidation.h"
#include "Settings/EngineMateSettings.h"

bool UNullReferenceValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject,
                                                              FDataValidationContext& InContext) const
{
    const UBlueprint* Blueprint = Cast<UBlueprint>(InObject);
    if (!Blueprint)
    {
        return false;
    }

    const UClass* GeneratedClass = Blueprint->GeneratedClass;
    return GeneratedClass && GeneratedClass->IsChildOf<AActor>();
}

EDataValidationResult UNullReferenceValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
    const UEngineMateSettings* EngineMateSettings = GetDefault<UEngineMateSettings>(); 
    UBlueprint* BP = Cast<UBlueprint>(InAsset);
    if (!BP || !BP->SimpleConstructionScript)
    {
        return EDataValidationResult::NotValidated;
    }

    // Get all component templates from the Blueprint's component tree
    const TArray<USCS_Node*>& AllNodes = BP->SimpleConstructionScript->GetAllNodes();
    
    for (const USCS_Node* Node : AllNodes)
    {
        UActorComponent* CompTemplate = Node->ComponentTemplate;
        if (!CompTemplate) continue;

        // Check Static Mesh Components
        if (const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(CompTemplate))
        {
            if (StaticMeshComponent->GetStaticMesh() == nullptr)
            {
                if (EngineMateSettings->bWarnOnEmptyStaticMeshComponents)
                {
                    Context.AddWarning(FText::Format(
            NSLOCTEXT("Validation", "NullStaticMesh",
                    "Blueprint '{0}': StaticMeshComponent '{1}' has no Static Mesh assigned."),
                    FText::FromString(BP->GetName()),
                    FText::FromString(Node->GetVariableName().ToString())
                        ));
                }
               
            }
        }
        
        // Check Skeletal Mesh Components
        else if (const USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(CompTemplate))
        {
            // Note: In newer versions, GetSkeletalMeshAsset() is preferred over GetSkeletalMesh()
            if (SkeletalMeshComponent->GetSkeletalMeshAsset() == nullptr)
            {
                if (EngineMateSettings->bWarnOnEmptySkeletalMeshComponents)
                {
                    Context.AddWarning(FText::Format(
                NSLOCTEXT("Validation", "NullSkeletalMesh",
                    "Blueprint '{0}': SkeletalMeshComponent '{1}' has no Skeletal Mesh assigned."),
                    FText::FromString(BP->GetName()),
                    FText::FromString(Node->GetVariableName().ToString())
                    ));
                }
               
            }
        }
    }

    // We always call AssetPasses and return Valid because you only want Warnings.
    // This allows the user to save despite the missing references.
    AssetPasses(InAsset);
    return EDataValidationResult::Valid;
}