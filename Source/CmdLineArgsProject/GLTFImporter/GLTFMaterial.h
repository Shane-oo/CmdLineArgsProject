// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GLTFParser.h"
#include "GLTFMaterial.generated.h"


struct FGlTFMaterialProperties
{
    FString Name;
    bool bDoubleSided = false;


    bool bTranslucent = false;
    bool bMasked = false;
    float AlphaCutOff = 0.5f;

    FVector4 Colour = FVector4(1.0f, 1.0f, 1.0f, 1.0f); // White
    float Roughness = 1.0f;
    float Metalness = 0.0f;

    FVector EmissiveColour = FVector(0.0f); // Black

    FGLTFParser::FGlTFTexture DiffuseTexture;
    FGLTFParser::FGlTFTexture NormalTexture;
    FGLTFParser::FGlTFTexture MetalnessRoughnessTexture;
    FGLTFParser::FGlTFTexture OcclusionTexture;
    FGLTFParser::FGlTFTexture EmissiveTexture;
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

private:
    static UMaterialInterface* GetBaseMaterialInterface(const FGlTFMaterialProperties& GlTFMaterialProperties);

    // todo down the track defs need a texture cache
    static UTexture2D* BuildTexture(FGLTFParser::FGlTFTexture GlTFTexture, UMaterialInterface* MaterialParent);
};
