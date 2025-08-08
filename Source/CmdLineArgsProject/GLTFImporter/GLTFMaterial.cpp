// Fill out your copyright notice in the Description page of Project Settings.


#include "GLTFMaterial.h"

// #region Public Methods

bool UGLTFMaterial::CreateMaterial(const FGlTFMaterialProperties& GlTFMaterialProperties)
{
    UMaterialInterface* BaseGlTFMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/M_GlTF_BaseMaterial"));
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


    // Compile the material
    DynamicMaterial->PostEditChange();

    return true;
}

// #endregion
