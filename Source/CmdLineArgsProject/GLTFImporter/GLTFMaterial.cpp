// Fill out your copyright notice in the Description page of Project Settings.


#include "GLTFMaterial.h"

// #region Private Methods

UMaterialInterface* UGLTFMaterial::GetBaseMaterialInterface(const FGlTFMaterialProperties& GlTFMaterialProperties)
{
    UMaterialInterface* BaseGlTFMaterial;

    if (GlTFMaterialProperties.bTranslucent && GlTFMaterialProperties.bDoubleSided)
    {
        BaseGlTFMaterial = LoadObject<UMaterialInterface>(
            nullptr, TEXT("/Game/M_GlTF_Translucent_DoubleSided_BaseMaterial")
        );
    }
    else if (GlTFMaterialProperties.bMasked && GlTFMaterialProperties.bDoubleSided)
    {
        BaseGlTFMaterial = LoadObject<UMaterialInterface>(
            nullptr, TEXT("/Game/M_GlTF_Masked_DoubleSided_BaseMaterial")
        );
    }
    else if (GlTFMaterialProperties.bTranslucent)
    {
        BaseGlTFMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/M_GlTF_Translucent_BaseMaterial"));
    }
    else if (GlTFMaterialProperties.bMasked)
    {
        BaseGlTFMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/M_GlTF_Masked_BaseMaterial"));
    }
    else if (GlTFMaterialProperties.bDoubleSided)
    {
        BaseGlTFMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/M_GlTF_DoubleSided_BaseMaterial"));
    }
    else
    {
        BaseGlTFMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/M_GlTF_BaseMaterial"));
    }

    return BaseGlTFMaterial;
}

// #endregion

// #region Public Methods

bool UGLTFMaterial::CreateMaterial(const FGlTFMaterialProperties& GlTFMaterialProperties)
{
    UMaterialInterface* BaseGlTFMaterial = GetBaseMaterialInterface(GlTFMaterialProperties);
    if (!BaseGlTFMaterial)
    {
        UE_LOG(LogTemp,
               Error,
               TEXT("FGLTFParser::AddMaterialToMaterialsMap::Error:: File: BaseGlTFMaterial Not Found!"));
        return false;
    }


    DynamicMaterial = UMaterialInstanceDynamic::Create(
        BaseGlTFMaterial,
        GetTransientPackage(),
        FName(GlTFMaterialProperties.Name)
    );

    DynamicMaterial->SetVectorParameterValue("glTFColour", GlTFMaterialProperties.Colour);
    DynamicMaterial->SetScalarParameterValue("glTFRoughness", GlTFMaterialProperties.Roughness);
    DynamicMaterial->SetScalarParameterValue("glTFMetalness", GlTFMaterialProperties.Metalness);

    DynamicMaterial->SetVectorParameterValue("gltfEmissiveColor", GlTFMaterialProperties.EmissiveColour);

    DynamicMaterial->SetScalarParameterValue("gltfAlphaCutOff", GlTFMaterialProperties.AlphaCutOff);

    // Compile the material
    DynamicMaterial->PostEditChange();

    return true;
}


// #endregion
