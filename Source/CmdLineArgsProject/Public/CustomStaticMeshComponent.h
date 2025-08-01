// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "CustomStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class CMDLINEARGSPROJECT_API UCustomStaticMeshComponent : public UStaticMeshComponent
{
    GENERATED_BODY()

public:
    UCustomStaticMeshComponent();

    void InitMesh(bool UseRedMaterial);

public:
    UPROPERTY(EditAnywhere)
    UMaterialInterface* Material;

    UPROPERTY(EditAnywhere)
    UStaticMesh* CubeMesh;

private:
};
