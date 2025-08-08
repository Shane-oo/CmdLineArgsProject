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

    bool CreateMesh(FString Name,
                    const TArray<FVector>& Vertices,
                    const TArray<int32>& Indices,
                    const TArray<FVector3f>& Normals,
                    const TArray<FVector2f>& TextureCoords0,
                    const FTransform& Transform);

private:
    FString Name;

    UPROPERTY(EditAnywhere)
    UMaterialInterface* Material;

    UPROPERTY(EditAnywhere)
    UStaticMesh* GlTFStaticMesh;

private:
    void CreateStaticMeshFromPrimitives(const FString& Name,
                                        TArray<FVector> Vertices,
                                        TArray<int32> Indices,
                                        TArray<FVector3f> Normals,
                                        TArray<FVector2f> TextureCoords0);
};
