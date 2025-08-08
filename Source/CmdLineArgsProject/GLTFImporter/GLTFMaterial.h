// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GLTFMaterial.generated.h"


struct FGlTFMaterialProperties
{
    FString Name;
    bool bDoubleSided = false;

    // OPAQUE - No transparency at all
    // BLEND - Uses full alpha blending
    // MASK - Uses alpha cutoff to discard pixels below a threshold
    FString AlphaMode = "OPAQUE";
    bool bTranslucent = false;
    bool bMasked = false;
    float AlphaCutOff = 0.5f;

    FVector4 Colour = FVector4(1.0f, 1.0f, 1.0f, 1.0f); // White
    float Roughness = 1.0f;
    float Metalness = 0.0f;
};

/**
 * 
 */
UCLASS()
class CMDLINEARGSPROJECT_API UGLTFMaterial : public UObject
{
    GENERATED_BODY()

public:
    bool CreateMaterial(const FGlTFMaterialProperties& GlTFMaterialProperties);

private:
    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial;

public:
    UMaterialInterface* GetMaterial() const
    {
        return DynamicMaterial;
    }
};
