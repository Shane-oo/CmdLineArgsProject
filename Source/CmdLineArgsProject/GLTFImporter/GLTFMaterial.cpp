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

UTexture2D* UGLTFMaterial::BuildTexture(FGLTFParser::FGlTFTexture GlTFTexture, UMaterialInterface* MaterialParent)
{
    if (!GlTFTexture.bIsValid || GlTFTexture.MipMappings.Num() == 0)
    {
        return nullptr;
    }

    UTexture2D* Texture = NewObject<UTexture2D>(MaterialParent, NAME_None, RF_Public);

    FTexturePlatformData* PlatformData = new FTexturePlatformData();
    PlatformData->SizeX = GlTFTexture.MipMappings[0].Width;
    PlatformData->SizeY = GlTFTexture.MipMappings[0].Height;
    PlatformData->PixelFormat = GlTFTexture.MipMappings[0].PixelFormat;

    Texture->SetPlatformData(PlatformData);

    // Load in Texture immediately
    Texture->NeverStream = true;

    for (const auto& GlTFMipMap : GlTFTexture.MipMappings)
    {
        FTexture2DMipMap* MipMap = new FTexture2DMipMap;
        PlatformData->Mips.Add(MipMap);
        MipMap->SizeX = GlTFMipMap.Width;
        MipMap->SizeY = GlTFMipMap.Height;

        MipMap->BulkData.Lock(LOCK_READ_WRITE);

        uint8* Data = MipMap->BulkData.Realloc(GlTFMipMap.Pixels.Num());

        FMemory::Memcpy(Data, GlTFMipMap.Pixels.GetData(), GlTFMipMap.Pixels.Num());

        MipMap->BulkData.Unlock();
    }

    Texture->SRGB = GlTFTexture.bIsSRGB;

    //    Texture->AddressX = Sampler.TileX;
    //    Texture->AddressY = Sampler.TileY;

    Texture->UpdateResource();

    return Texture;
}

// #endregion

// #region Public Methods

bool UGLTFMaterial::CreateMaterial(const FGlTFMaterialProperties& GlTFMaterialProperties)
{
    UMaterialInterface* BaseGlTFMaterial = GetBaseMaterialInterface(GlTFMaterialProperties);
    if (!BaseGlTFMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("FGLTFParser::CreateMaterial::Error::BaseGlTFMaterial Not Found!"));
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

    // leave out for now
    //DynamicMaterial->SetVectorParameterValue("gltfEmissiveColor", GlTFMaterialProperties.EmissiveColour);

    DynamicMaterial->SetScalarParameterValue("gltfAlphaCutOff", GlTFMaterialProperties.AlphaCutOff);

    if (const auto DiffuseTexture = BuildTexture(GlTFMaterialProperties.DiffuseTexture, DynamicMaterial))
    {
        DynamicMaterial->SetScalarParameterValue("useGlTFDiffuseTexture", 1.0f);
        DynamicMaterial->SetTextureParameterValue("glTFDiffuseTexture", DiffuseTexture);
        DynamicMaterial->SetScalarParameterValue("glTFDiffuseTextureCoord",
                                                 GlTFMaterialProperties.DiffuseTexture.TextureCoord);
    }

    if (const auto NormalTexture = BuildTexture(GlTFMaterialProperties.NormalTexture, DynamicMaterial))
    {
        DynamicMaterial->SetScalarParameterValue("useGlTFNormalTexture", 1.0f);
        DynamicMaterial->SetTextureParameterValue("glTFNormalTexture", NormalTexture);
        DynamicMaterial->SetScalarParameterValue("glTFNormalTextureCoord",
                                                 GlTFMaterialProperties.NormalTexture.TextureCoord);
    }

    if (const auto MetalnessRoughnessTexture = BuildTexture(GlTFMaterialProperties.MetalnessRoughnessTexture,
                                                            DynamicMaterial))
    {
        DynamicMaterial->SetScalarParameterValue("useGlTFMetalnessRoughnessTexture", 1.0f);
        DynamicMaterial->SetTextureParameterValue("glTFMetalnessRoughnessTexture", MetalnessRoughnessTexture);
        DynamicMaterial->SetScalarParameterValue("glTFMetalnessRoughnessTextureCoord",
                                                 GlTFMaterialProperties.MetalnessRoughnessTexture.TextureCoord);
    }

    // Compile the material
    DynamicMaterial->PostEditChange();

    return true;
}


// #endregion
