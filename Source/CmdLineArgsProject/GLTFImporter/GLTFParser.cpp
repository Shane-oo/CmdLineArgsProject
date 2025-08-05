//
// Created by Shane on 5/8/25.
//

#include "GLTFParser.h"

// #region Constructors

FGLTFParser::FGLTFParser()
{
}

// #endregion

// #region Destructors

FGLTFParser::~FGLTFParser()
{
}

// #endregion

// #region Public Methods

void FGLTFParser::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObjects(StaticMeshesCache);
    //Collector.AddReferencedObjects(MaterialsCache);
    //Collector.AddReferencedObjects(TexturesCache);
    //Collector.AddReferencedObjects(MaterialsNameCache);
    //Collector.AddReferencedObjects(MetallicRoughnessMaterialsMap);
    //Collector.AddReferencedObjects(SpecularGlossinessMaterialsMap);
}


TSharedPtr<FGLTFParser> FGLTFParser::CreateFromFileName(const FString& FileName)
{
    if (!FPaths::FileExists(FileName))
    {
        UE_LOG(LogTemp, Error, TEXT("FGLTFParser::Create::Error:: File: %s Does Not Exist"), *FileName);
        return nullptr;
    }

    TArray64<uint8> Buffer;
    if (!FFileHelper::LoadFileToArray(Buffer, *FileName))
    {
        UE_LOG(LogTemp, Error, TEXT("FGLTFParser::Create::Error:: Unable to load File: %s"), *FileName);
        return nullptr;
    }

    TSharedPtr<FGLTFParser> Parser = CreateFromRawData(Buffer.GetData(), Buffer.Num());

    return Parser;
}

TSharedPtr<FGLTFParser> FGLTFParser::CreateFromRawData(const uint8* DataPtr, int64 DataNum)
{
    // glb
    if (DataNum > 20)
    {
        if (DataPtr[0] == 0x67 &&
            DataPtr[1] == 0x6C &&
            DataPtr[2] == 0x54 &&
            DataPtr[3] == 0x46)
        {
            UE_LOG(LogTemp, Display, TEXT("FGLTFParser::CreateFromRawData::Display:: This file is a glb"));
        }
    }

    // glTF
    if (DataNum > 0 && DataNum <= INT32_MAX)
    {
        FString JsonData;
        FFileHelper::BufferToString(JsonData, DataPtr, static_cast<int32>(DataNum));

        UE_LOG(LogTemp, Display, TEXT("FGLTFParser::CreateFromRawData::Display:: This file is a glTF"));
    }

    return nullptr;
}

// #endregion
