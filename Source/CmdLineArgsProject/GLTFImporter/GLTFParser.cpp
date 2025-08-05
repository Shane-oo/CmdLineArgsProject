//
// Created by Shane on 5/8/25.
//

#include "GLTFParser.h"

// #region Constructors

FGLTFParser::FGLTFParser(const TSharedPtr<FJsonObject>& JsonObject)
{
    Root = JsonObject;

    if (IsInGameThread())
    {
        UE_LOG(LogTemp, Display, TEXT("FGLTFParser::FGLTFParser::Display:: In Game Thread load mats"));
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("FGLTFParser::FGLTFParser::Display:: Not In Game Thread load mats when we are"));
    }
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
    // UObjects to keep alive

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

TSharedPtr<FGLTFParser> FGLTFParser::CreateFromRawData(const uint8* DataPtr, const int64 DataNum)
{
    // glb
    if (DataNum > 20)
    {
        if (DataPtr[0] == 0x67 &&
            DataPtr[1] == 0x6C &&
            DataPtr[2] == 0x54 &&
            DataPtr[3] == 0x46)
        {
            return CreateFromBinaryData(DataPtr, DataNum);
        }
    }

    // glTF
    if (DataNum > 0 && DataNum <= INT32_MAX)
    {
        FString JsonData;
        FFileHelper::BufferToString(JsonData, DataPtr, static_cast<int32>(DataNum));
        return CreateFromString(JsonData);
    }

    return nullptr;
}

TSharedPtr<FGLTFParser> FGLTFParser::CreateFromBinaryData(const uint8* DataPtr, const int64 DataNum)
{
    FString JsonData;
    //BinaryBuffer is the chunk of raw, unprocessed binary data from the GLB file that matches the "BIN" section.
    //will use it to resolve buffer views in the glTF JSON, things like vertex positions, normals, etc
    TArray64<uint8> BinaryBuffer;

    bool bJsonFound = false;
    bool bBinaryFound = false;
    int64 BlobIndex = 12;

    while (BlobIndex < DataNum)
    {
        if (BlobIndex + 8 > DataNum)
        {
            return nullptr;
        }

        const uint32* ChunkLength = reinterpret_cast<const uint32*>(&DataPtr[BlobIndex]);
        const uint32* ChunkType = reinterpret_cast<const uint32*>(&DataPtr[BlobIndex + 4]);

        BlobIndex += 8;

        if ((BlobIndex + *ChunkLength) > DataNum)
        {
            return nullptr;
        }

        //Type = "JSON" (0x4E4F534A)
        if (*ChunkType == 0x4E4F534A && !bJsonFound)
        {
            bJsonFound = true;
            FFileHelper::BufferToString(JsonData, &DataPtr[BlobIndex], *ChunkLength);
        }
        //Type = "BIN" (0x004E4942)
        else if (*ChunkType == 0x004E4942 && !bBinaryFound)
        {
            bBinaryFound = true;
            BinaryBuffer.Append(&DataPtr[BlobIndex], *ChunkLength);
        }

        BlobIndex += *ChunkLength;
    }

    if (!bJsonFound)
    {
        return nullptr;
    }

    TSharedPtr<FGLTFParser> Parser = CreateFromString(JsonData);

    if (Parser)
    {
        if (bBinaryFound)
        {
            Parser->SetBinaryBuffer(BinaryBuffer);
        }
    }

    return Parser;
}

TSharedPtr<FGLTFParser> FGLTFParser::CreateFromString(const FString& GlTFJsonData)
{
    TSharedPtr<FJsonValue> RootValue;

    if (const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(GlTFJsonData);
        !FJsonSerializer::Deserialize(JsonReader, RootValue))
    {
        return nullptr;
    }

    const TSharedPtr<FJsonObject> JsonObject = RootValue->AsObject();
    if (!JsonObject || !JsonObject.IsValid())
    {
        return nullptr;
    }

    TSharedPtr<FGLTFParser> Parser = MakeShared<FGLTFParser>(JsonObject);

    return Parser;
}

// #endregion
