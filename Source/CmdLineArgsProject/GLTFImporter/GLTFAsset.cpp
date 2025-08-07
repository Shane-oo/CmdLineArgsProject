//
// Created by Shane on 5/8/25.
//

#include "GLTFAsset.h"
#include "GLTFParser.h"

// #region Public Methods

bool UGLTFAsset::LoadFromFileName(const FString& FileName)
{
    if (Parser)
    {
        UE_LOG(LogTemp, Error, TEXT("UGLTFAsset::LoadFromFileName::Error:: Parser Aleady Exists"));
        return false;
    }

    Parser = FGLTFParser::CreateFromFileName(FileName);

    return Parser != nullptr;
}

bool UGLTFAsset::LoadModel()
{
    if (!Parser)
    {
        UE_LOG(LogTemp, Error,
               TEXT("UGLTFAsset::LoadModel::Error:: No glTF File Loaded yet. Call LoadFromFileName()"));

        return false;
    }

    return Parser->LoadScene(this);
}

// #endregion
