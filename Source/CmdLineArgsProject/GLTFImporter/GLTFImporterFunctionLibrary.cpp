//
// Created by Shane on 5/8/25.
//

#include "GLTFImporterFunctionLibrary.h"

#include "GLTFAsset.h"

// #region Public Methods

UGLTFAsset* UGLTFImporterFunctionLibrary::ImportGlTF(const FString& FileName)
{
    UGLTFAsset* Asset = NewObject<UGLTFAsset>();
    if (!Asset)
    {
        return nullptr;
    }

    if (const bool AssetLoaded = Asset->LoadFromFileName(FileName); !AssetLoaded)
    {
        return nullptr;
    }

    return Asset;
}

// #endregion
