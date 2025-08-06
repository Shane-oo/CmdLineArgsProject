//
// Created by Shane on 5/8/25.
//

#include "GLTFParser.h"

// #region Private Methods

void FGLTFParser::CheckExtensionsRequired() const
{
    if (Root)
    {
        TArray<FString> ExtensionsRequired;
        Root->TryGetStringArrayField(TEXT("extensionsRequired"), ExtensionsRequired);
        if (ExtensionsRequired.Contains("KHR_draco_mesh_compression"))
        {
            UE_LOG(LogTemp, Error, TEXT("FGLTFParser::FGLTFParser::Error:: Draco Compression Not Supported"));
        }
    }
}

bool FGLTFParser::CheckJsonIndex(const TSharedPtr<FJsonObject>& JsonObject,
                                 const FString& FieldName,
                                 const int32 Index,
                                 TArray<TSharedRef<FJsonValue>>& JsonItems)
{
    if (Index < 0)
    {
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* JsonArray;
    if (!JsonObject->TryGetArrayField(FieldName, JsonArray))
    {
        return false;
    }

    if (Index >= JsonArray->Num())
    {
        return false;
    }

    for (const TSharedPtr<FJsonValue>& JsonItem : *JsonArray)
    {
        JsonItems.Add(JsonItem.ToSharedRef());
    }

    return true;
}

TSharedPtr<FJsonObject> FGLTFParser::GetJsonObjectFromIndex(const TSharedPtr<FJsonObject>& JsonObject,
                                                            const FString& FieldName,
                                                            const int32 Index)
{
    TArray<TSharedRef<FJsonValue>> JsonArray;

    if (!CheckJsonIndex(JsonObject, FieldName, Index, JsonArray))
    {
        return nullptr;
    }

    return JsonArray[Index]->AsObject();
}

TSharedPtr<FJsonObject> FGLTFParser::GetJsonObjectFromRootIndex(const FString& FieldName, const int32 Index) const
{
    return GetJsonObjectFromIndex(Root, FieldName, Index);
}

int64 FGLTFParser::GetJsonObjectIndex(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
    if (int64 Index; JsonObject->TryGetNumberField(FieldName, Index))
    {
        return Index;
    }

    return -1;
}

FString FGLTFParser::GetJsonObjectString(const TSharedPtr<FJsonObject>& JsonObject,
                                         const FString& FieldName,
                                         const FString& DefaultValue)
{
    FString Value;
    if (!JsonObject->TryGetStringField(FieldName, Value))
    {
        return DefaultValue;
    }

    return Value;
}

template <int32 Num, typename T>
bool FGLTFParser::GetJsonVector(const TArray<TSharedPtr<FJsonValue>>* JsonValues, T& Value)
{
    if (JsonValues->Num() != Num)
    {
        // inputted incorrect size of vector i.e put 4 values for a Vector3D
        return false;
    }

    for (int32 i = 0; i < Num; i++)
    {
        if (!(*JsonValues)[i]->TryGetNumber(Value[i]))
        {
            return false;
        }
    }

    return true;
}

bool FGLTFParser::FillJsonMatrix(const TArray<TSharedPtr<FJsonValue>>* JsonValues, FMatrix& Matrix)
{
    // Size of Matrix is 16 (4x4)
    if (JsonValues->Num() != 16)
    {
        return false;
    }

    for (int32 i = 0; i < 16; i++)
    {
        float Value;
        if (!(*JsonValues)[i]->TryGetNumber(Value))
        {
            return false;
        }

        Matrix.M[i / 4][i % 4] = Value;
    }

    return true;
}

bool FGLTFParser::LoadNode(TSharedPtr<FJsonObject> JsonNode, int32 NodeIndex)
{
    auto Name = GetJsonObjectString(JsonNode, "name", FString::FromInt(NodeIndex));

    FVector Translation(0, 0, 0);
    FQuat Rotation(0, 0, 0, 1);
    FVector Scale(1, 1, 1);

    FMatrix Transform = FMatrix::Identity;

    if (const TArray<TSharedPtr<FJsonValue>>* JsonTranslationValues;
        JsonNode->TryGetArrayField(TEXT("translation"), JsonTranslationValues))
    {
        if (!GetJsonVector<3>(JsonTranslationValues, Translation))
        {
            return false;
        }
    }

    if (const TArray<TSharedPtr<FJsonValue>>* JsonRotationValues;
        JsonNode->TryGetArrayField(TEXT("rotation"), JsonRotationValues))
    {
        FVector4 Vector;
        if (!GetJsonVector<4>(JsonRotationValues, Vector))
        {
            return false;
        }

        Rotation = {Vector.X, Vector.Y, Vector.Z, Vector.W};
    }

    if (const TArray<TSharedPtr<FJsonValue>>* JsonScaleValues;
        JsonNode->TryGetArrayField(TEXT("scale"), JsonScaleValues))
    {
        if (!GetJsonVector<3>(JsonScaleValues, Scale))
        {
            return false;
        }
    }


    if (const TArray<TSharedPtr<FJsonValue>>* JsonMatrixValues;
        JsonNode->TryGetArrayField(TEXT("matrix"), JsonMatrixValues))
    {
        if (!FillJsonMatrix(JsonMatrixValues, Transform))
        {
            return false;
        }
    }
    else
    {
        const auto RotationMatrix = Rotation.ToMatrix();

        const auto XAxis = RotationMatrix.GetScaledAxis(EAxis::X) * Scale.X;
        const auto YAxis = RotationMatrix.GetScaledAxis(EAxis::Y) * Scale.Y;
        const auto ZAxis = RotationMatrix.GetScaledAxis(EAxis::Z) * Scale.Z;

        Transform = FMatrix(XAxis, YAxis, ZAxis, Translation);
    }


    //UE_LOG(LogTemp, Log, TEXT("Node %d Transform:\n%s"), NodeIndex, *Transform.ToString());

    if (const TArray<TSharedPtr<FJsonValue>>* JsonChildren;
        JsonNode->TryGetArrayField(TEXT("children"), JsonChildren))
    {
        for (int i = 0; i < JsonChildren->Num(); i++)
        {
            int64 ChildIndex;
            if (!(*JsonChildren)[i]->TryGetNumber(ChildIndex))
            {
                return false;
            }

            const TSharedPtr<FJsonObject> JsonNodeObject = GetJsonObjectFromRootIndex("nodes", ChildIndex);

            if (!JsonNodeObject)
            {
                return false;
            }

            if (!LoadNode(JsonNodeObject, ChildIndex))
            {
                return false;
            }
        }
    }


    if (const auto MeshIndex = GetJsonObjectIndex(JsonNode, TEXT("mesh"));
        MeshIndex != -1)
    {
        TSharedPtr<FJsonObject> JsonMeshObject = GetJsonObjectFromRootIndex("meshes", MeshIndex);
        if (!JsonMeshObject)
        {
            return false;
        }

        // Load Primitives
        int PrimitiveCount = 0;
        const TArray<TSharedPtr<FJsonValue>>* JsonPrimitives;
        if (!JsonMeshObject->TryGetArrayField(TEXT("primitives"), JsonPrimitives))
        {
            return false;
        }

        for (auto JsonPrimitive : *JsonPrimitives)
        {
            TSharedPtr<FJsonObject> JsonPrimitiveObject = JsonPrimitive->AsObject();
            if (!JsonPrimitiveObject)
            {
                return false;
            }
            //NOTE: Assuming always Triangles!

            const TSharedPtr<FJsonObject>* JsonAttributesObject;
            if (!JsonPrimitiveObject->TryGetObjectField(TEXT("attributes"), JsonAttributesObject))
            {
                return false;
            }

            // Vertices
            int64 PositionAccessorIndex;
            if (!(*JsonAttributesObject)->TryGetNumberField(TEXT("POSITION"), PositionAccessorIndex))
            {
                return false;
            }

            UE_LOG(LogTemp, Log, TEXT("POSITION accessor index: %d"), PositionAccessorIndex);
        }
    }

    return true;
}

// #endregion

// #region Constructors

FGLTFParser::FGLTFParser(const TSharedPtr<FJsonObject>& JsonObject)
{
    Root = JsonObject;

    CheckExtensionsRequired();
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

bool FGLTFParser::LoadScene()
{
    // Load Materials...

    // Load All scenes => Usually just 1 with an index of 0
    const TArray<TSharedPtr<FJsonValue>>* JsonScenes;
    if (!Root->TryGetArrayField(TEXT("scenes"), JsonScenes))
    {
        // No Scenes, empty gltf
        return false;
    }

    for (int32 SceneIndex = 0; SceneIndex < JsonScenes->Num(); SceneIndex++)
    {
        TSharedPtr<FJsonObject> JsonSceneObject = GetJsonObjectFromRootIndex("scenes", SceneIndex);
        if (!JsonSceneObject)
        {
            return false;
        }

        const TArray<TSharedPtr<FJsonValue>>* JsonSceneNodes;
        if (JsonSceneObject->TryGetArrayField(TEXT("nodes"), JsonSceneNodes))
        {
            for (int32 Index = 0; Index < JsonSceneNodes->Num(); Index++)
            {
                TSharedPtr<FJsonObject> JsonNodeObject = GetJsonObjectFromRootIndex("nodes", Index);

                if (!JsonNodeObject)
                {
                    return false;
                }

                if (!LoadNode(JsonNodeObject, Index))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

// #endregion
