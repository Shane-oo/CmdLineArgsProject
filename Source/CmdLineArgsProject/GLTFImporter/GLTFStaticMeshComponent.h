// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "GLTFStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class CMDLINEARGSPROJECT_API UGLTFStaticMeshComponent : public UStaticMeshComponent
{
    GENERATED_BODY()

public:
    UGLTFStaticMeshComponent();

    bool Init(FString Name, TArray<FVector> Vertices, TArray<int32> Indices);

private:
    FString Name;

    UPROPERTY(EditAnywhere)
    UMaterialInterface* Material;

    UPROPERTY(EditAnywhere)
    UStaticMesh* GlTFStaticMesh;
};
