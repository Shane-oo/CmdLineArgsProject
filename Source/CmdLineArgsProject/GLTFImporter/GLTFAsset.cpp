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

// #endregion
