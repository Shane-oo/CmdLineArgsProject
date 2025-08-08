//
// Created by Shane on 5/8/25.
//

#include "GLTFParser.h"

#include "GLTFAsset.h"
#include "GLTFMaterial.h"
#include "GLTFStaticMeshComponent.h"

// #region Private Methods

class UGLTFStaticMeshComponent;

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

template <int32 Num, typename T>
bool FGLTFParser::GetJsonVector(const TSharedPtr<FJsonValue>* JsonValue, T& Value)
{
    for (int32 i = 0; i < Num; i++)
    {
        if (!JsonValue[i]->TryGetNumber(Value[i]))
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

bool FGLTFParser::GetBufferView(const int32 BufferViewIndex, FBuffer& OutBuffer)
{
    const TSharedPtr<FJsonObject> JsonBufferViewObject = GetJsonObjectFromRootIndex(
        "bufferViews", BufferViewIndex);
    if (!JsonBufferViewObject)
    {
        return false;
    }

    int64 BufferIndex;
    if (!JsonBufferViewObject->TryGetNumberField(TEXT("buffer"), BufferIndex))
    {
        return false;
    }

    FBuffer TotalBuffer;
    if (!GetBuffer(BufferIndex, TotalBuffer))
    {
        return false;
    }

    int64 ByteLength;
    if (!JsonBufferViewObject->TryGetNumberField(TEXT("byteLength"), ByteLength))
    {
        return false;
    }

    int64 ByteOffset;
    if (!JsonBufferViewObject->TryGetNumberField(TEXT("byteOffset"), ByteOffset))
    {
        return false;
    }

    // byteStride? -> Not supporting Interleaved right now, 

    if (ByteOffset + ByteLength > TotalBuffer.Num)
    {
        return false;
    }

    OutBuffer.Data = TotalBuffer.Data;
    OutBuffer.Num = ByteLength;
    OutBuffer.ByteOffset = ByteOffset;

    return true;
}

bool FGLTFParser::GetBuffer(const int32 BufferIndex, FBuffer& OutBuffer)
{
    // if glb then data is in the BinaryBuffer
    // else if gltf look at Json Buffers, will be inside the gltf file or separate with a reference to the .bin File

    if (BufferIndex == 0 && BinaryBuffer.Num() > 0)
    {
        OutBuffer.Data = BinaryBuffer.GetData();
        OutBuffer.Num = BinaryBuffer.Num();
        return true;
    }

    // todo look at the JsonBuffers (non-glb)
    return false;
}

bool FGLTFParser::GetVertices(const TSharedPtr<FJsonObject>* JsonAttributesObject, TArray<FVector>& Vertices)
{
    int64 PositionAccessorIndex;
    if (!(*JsonAttributesObject)->TryGetNumberField(TEXT("POSITION"), PositionAccessorIndex))
    {
        return false;
    }

    const TSharedPtr<FJsonObject> JsonPositionAccessorObject = GetJsonObjectFromRootIndex(
        "accessors",
        PositionAccessorIndex);
    if (!JsonPositionAccessorObject)
    {
        return false;
    }

    int64 PositionCount;
    if (!JsonPositionAccessorObject->TryGetNumberField(TEXT("count"), PositionCount))
    {
        return false;
    }

    int64 PositionBufferViewIndex;
    int64 PositionByteOffset = 0;
    if (!JsonPositionAccessorObject->TryGetNumberField(TEXT("bufferView"), PositionBufferViewIndex))
    {
        return false;
    }
    JsonPositionAccessorObject->TryGetNumberField(TEXT("byteOffset"), PositionByteOffset);

    FBuffer PositionsBuffer;
    if (!GetBufferView(PositionBufferViewIndex, PositionsBuffer))
    {
        return false;
    }

    const auto* PositionsData = reinterpret_cast<const float*>(
        &PositionsBuffer.Data[PositionsBuffer.ByteOffset + PositionByteOffset]
    );


    Vertices.Reserve(PositionCount);

    for (int i = 0; i < PositionCount; i++)
    {
        FVector Position(PositionsData[i * 3], PositionsData[i * 3 + 1], PositionsData[i * 3 + 2]);
        Vertices.Add(Position);
    }

    return true;
}

// Note: Not the Accessor it's in the Primitive
void FGLTFParser::GetIndices(const TSharedPtr<FJsonObject>* JsonPrimitiveObject, TArray<int32>& Indices)
{
    int64 IndicesAccessorIndex;
    if (!(*JsonPrimitiveObject)->TryGetNumberField(TEXT("indices"), IndicesAccessorIndex))
    {
        return;
    }

    const TSharedPtr<FJsonObject> JsonIndicesAccessorObject = GetJsonObjectFromRootIndex(
        "accessors",
        IndicesAccessorIndex);
    if (!JsonIndicesAccessorObject)
    {
        return;
    }

    int64 IndicesCount;
    if (!JsonIndicesAccessorObject->TryGetNumberField(TEXT("count"), IndicesCount))
    {
        return;
    }

    int64 IndicesBufferViewIndex;
    int64 IndicesByteOffset = 0;
    if (!JsonIndicesAccessorObject->TryGetNumberField(TEXT("bufferView"), IndicesBufferViewIndex))
    {
        return;
    }
    JsonIndicesAccessorObject->TryGetNumberField(TEXT("byteOffset"), IndicesByteOffset);

    FBuffer IndicesBuffer;
    if (!GetBufferView(IndicesBufferViewIndex, IndicesBuffer))
    {
        return;
    }


    Indices.Reserve(IndicesCount);

    int64 ComponentType;
    if (!JsonIndicesAccessorObject->TryGetNumberField(TEXT("componentType"), ComponentType))
    {
        return;
    }

    const auto* IndicesBufferData = &IndicesBuffer.Data[IndicesBuffer.ByteOffset + IndicesByteOffset];

    switch (ComponentType)
    {
    case Gltf_Unsigned_Byte:
        {
            const auto* IndicesUint8 = IndicesBufferData;
            for (size_t i = 0; i < IndicesCount; i++)
            {
                Indices.Add(IndicesUint8[i]);
            }
            break;
        }
    case Gltf_Unsigned_Short:
        {
            const auto* IndicesUint16 = reinterpret_cast<const uint16*>(IndicesBufferData);
            for (size_t i = 0; i < IndicesCount; i++)
            {
                Indices.Add(IndicesUint16[i]);
            }
            break;
        }
    case Gltf_Unsigned_INT:
        {
            const auto* IndicesUint32 = reinterpret_cast<const uint32*>(IndicesBufferData);
            for (size_t i = 0; i < IndicesCount; i++)
            {
                Indices.Add(IndicesUint32[i]);
            }
            break;
        }
    default:
        {
            UE_LOG(LogTemp, Error, TEXT("FGLTFParser::GetIndices::Error:: Invalid index accessor %d Not Supported."),
                   ComponentType);
        }
    }
}

void FGLTFParser::GetNormals(const TSharedPtr<FJsonObject>* JsonAttributesObject, TArray<FVector3f>& Normals)
{
    int64 NormalAccessorIndex;
    if (!(*JsonAttributesObject)->TryGetNumberField(TEXT("NORMAL"), NormalAccessorIndex))
    {
        return;
    }

    const TSharedPtr<FJsonObject> JsonNormalAccessorObject = GetJsonObjectFromRootIndex(
        "accessors",
        NormalAccessorIndex);
    if (!JsonNormalAccessorObject)
    {
        return;
    }

    int64 NormalCount;
    if (!JsonNormalAccessorObject->TryGetNumberField(TEXT("count"), NormalCount))
    {
        return;
    }


    int64 NormalBufferViewIndex;
    int64 NormalByteOffset = 0;
    if (!JsonNormalAccessorObject->TryGetNumberField(TEXT("bufferView"), NormalBufferViewIndex))
    {
        return;
    }
    JsonNormalAccessorObject->TryGetNumberField(TEXT("byteOffset"), NormalByteOffset);

    FBuffer NormalBuffer;
    if (!GetBufferView(NormalBufferViewIndex, NormalBuffer))
    {
        return;
    }

    const auto* NormalsData = reinterpret_cast<const float*>(
        &NormalBuffer.Data[NormalBuffer.ByteOffset + NormalByteOffset]
    );

    Normals.Reserve(NormalCount);

    for (int i = 0; i < NormalCount; i++)
    {
        FVector3f Normal(NormalsData[i * 3], NormalsData[i * 3 + 1], NormalsData[i * 3 + 2]);
        Normals.Add(Normal);
    }
}

void FGLTFParser::GetTangents(const TSharedPtr<FJsonObject>* JsonAttributesObject, TArray<FVector4f>& Tangents)
{
    int64 TangentAccessorIndex;
    if (!(*JsonAttributesObject)->TryGetNumberField(TEXT("TANGENT"), TangentAccessorIndex))
    {
        return;
    }

    const TSharedPtr<FJsonObject> JsonTangentAccessorObject = GetJsonObjectFromRootIndex(
        "accessors",
        TangentAccessorIndex);
    if (!JsonTangentAccessorObject)
    {
        return;
    }


    int64 TangentCount;
    if (!JsonTangentAccessorObject->TryGetNumberField(TEXT("count"), TangentCount))
    {
        return;
    }

    int64 TangentBufferViewIndex;
    int64 TangentByteOffset = 0;
    if (!JsonTangentAccessorObject->TryGetNumberField(TEXT("bufferView"), TangentBufferViewIndex))
    {
        return;
    }
    JsonTangentAccessorObject->TryGetNumberField(TEXT("byteOffset"), TangentByteOffset);

    FBuffer TangentBuffer;
    if (!GetBufferView(TangentBufferViewIndex, TangentBuffer))
    {
        return;
    }

    const auto* TangentData = reinterpret_cast<const float*>(
        &TangentBuffer.Data[TangentBuffer.ByteOffset + TangentByteOffset]
    );

    Tangents.Reserve(TangentCount);

    for (int i = 0; i < TangentCount; i++)
    {
        FVector4f Tangent(TangentData[i * 4], TangentData[i * 4 + 1], TangentData[i * 4 + 2], TangentData[i * 4 + 3]);
        Tangents.Add(Tangent);
    }
}

void FGLTFParser::GetTextureCoords(const TSharedPtr<FJsonObject>* JsonAttributesObject,
                                   TArray<FVector2f>& TextureCoords,
                                   const FString& TextureCoordField)
{
    int64 TextureCoord0AccessorIndex;
    if (!(*JsonAttributesObject)->TryGetNumberField(TextureCoordField, TextureCoord0AccessorIndex))
    {
        return;
    }

    const TSharedPtr<FJsonObject> JsonTextureCoords0AccessorObject = GetJsonObjectFromRootIndex(
        "accessors",
        TextureCoord0AccessorIndex);
    if (!JsonTextureCoords0AccessorObject)
    {
        return;
    }


    int64 TextureCoords0Count;
    if (!JsonTextureCoords0AccessorObject->TryGetNumberField(TEXT("count"), TextureCoords0Count))
    {
        return;
    }

    int64 TextureCoords0BufferViewIndex;
    int64 TextureCoords0ByteOffset = 0;
    if (!JsonTextureCoords0AccessorObject->TryGetNumberField(TEXT("bufferView"), TextureCoords0BufferViewIndex))
    {
        return;
    }
    JsonTextureCoords0AccessorObject->TryGetNumberField(TEXT("byteOffset"), TextureCoords0ByteOffset);


    FBuffer TextureCoords0Buffer;
    if (!GetBufferView(TextureCoords0BufferViewIndex, TextureCoords0Buffer))
    {
        return;
    }

    const auto* TextureCoords0Data = reinterpret_cast<const float*>(
        &TextureCoords0Buffer.Data[TextureCoords0Buffer.ByteOffset + TextureCoords0ByteOffset]
    );

    TextureCoords.Reserve(TextureCoords0Count);

    for (int i = 0; i < TextureCoords0Count; i++)
    {
        FVector2f TextureCoord(TextureCoords0Data[i * 2], TextureCoords0Data[i * 2 + 1]);
        TextureCoords.Add(TextureCoord);
    }
}

void FGLTFParser::GetVertexColours(const TSharedPtr<FJsonObject>* JsonAttributesObject, TArray<FVector3f>& Colours)
{
    int64 ColourAccessorIndex;
    if (!(*JsonAttributesObject)->TryGetNumberField(TEXT("COLOR_0"), ColourAccessorIndex))
    {
        return;
    }

    const TSharedPtr<FJsonObject> JsonColourAccessorObject = GetJsonObjectFromRootIndex(
        "accessors",
        ColourAccessorIndex);
    if (!JsonColourAccessorObject)
    {
        return;
    }

    int64 ColourCount;
    if (!JsonColourAccessorObject->TryGetNumberField(TEXT("count"), ColourCount))
    {
        return;
    }

    int64 ColourBufferViewIndex;
    int64 ColourByteOffset = 0;
    if (!JsonColourAccessorObject->TryGetNumberField(TEXT("bufferView"), ColourBufferViewIndex))
    {
        return;
    }
    JsonColourAccessorObject->TryGetNumberField(TEXT("byteOffset"), ColourByteOffset);

    FBuffer ColourBuffer;
    if (!GetBufferView(ColourBufferViewIndex, ColourBuffer))
    {
        return;
    }

    Colours.Reserve(ColourCount);

    int64 ComponentType;
    if (!JsonColourAccessorObject->TryGetNumberField(TEXT("componentType"), ComponentType))
    {
        return;
    }

    const auto* ColourBufferData = &ColourBuffer.Data[ColourBuffer.ByteOffset + ColourByteOffset];

    // what if colours_0 is a vec4 not a vec3. Accepting this for now as we dont normally use vertex colours anyone
    // just adding for completeness
    switch (ComponentType)
    {
    case Gltf_Float:
        {
            const auto* ColoursFloat = reinterpret_cast<const float*>(ColourBufferData);
            for (size_t i = 0; i < ColourCount; i++)
            {
                FVector3f Colour(ColoursFloat[i * 3], ColoursFloat[i * 3 + 1], ColoursFloat[i * 3 + 2]);
                Colours.Add(Colour);
            }
            break;
        }

    case Gltf_Unsigned_Byte:
        {
            const auto* ColoursUint8 = ColourBufferData;
            for (size_t i = 0; i < ColourCount; i++)
            {
                FVector3f Colour(ColoursUint8[i * 3], ColoursUint8[i * 3 + 1], ColoursUint8[i * 3 + 2]);
                Colours.Add(Colour);
            }
            break;
        }

    case Gltf_Unsigned_Short:
        {
            const auto* ColoursUint16 = reinterpret_cast<const uint16*>(ColourBufferData);
            for (size_t i = 0; i < ColourCount; i++)
            {
                FVector3f Colour(ColoursUint16[i * 3], ColoursUint16[i * 3 + 1], ColoursUint16[i * 3 + 2]);
                Colours.Add(Colour);
            }
            break;
        }
    default:
        {
            UE_LOG(LogTemp,
                   Error,
                   TEXT("FGLTFParser::GetVertexColours::Error:: Invalid vertex colour accessor %d Not Supported."),
                   ComponentType);
        }
    }
}


bool FGLTFParser::LoadNode(TSharedPtr<FJsonObject> JsonNode, int32 NodeIndex)
{
    auto Name = GetJsonObjectString(JsonNode, "name", FString::FromInt(NodeIndex));

    FVector Translation(0, 0, 0);
    FQuat Rotation(0, 0, 0, 1);
    FVector Scale(1, 1, 1);

    FTransform Transform = FTransform::Identity;

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
        FMatrix Matrix = FMatrix::Identity;
        if (!FillJsonMatrix(JsonMatrixValues, Matrix))
        {
            return false;
        }

        Transform = FTransform(Matrix);
    }
    else
    {
        Transform = FTransform(Rotation, Translation, Scale);
    }

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

            TArray<FVector> Vertices;
            if (!GetVertices(JsonAttributesObject, Vertices))
            {
                return false;
            }

            TArray<int32> Indices;
            GetIndices(&JsonPrimitiveObject, Indices);

            TArray<FVector3f> Normals;
            GetNormals(JsonAttributesObject, Normals);

            // We don't do tangents right now, might need to compute using the funny algorithm
            TArray<FVector4f> Tangents;
            GetTangents(JsonAttributesObject, Tangents);

            TArray<FVector2f> TextureCoords0;
            GetTextureCoords(JsonAttributesObject, TextureCoords0, "TEXCOORD_0");

            TArray<FVector2f> TextureCoords1;
            GetTextureCoords(JsonAttributesObject, TextureCoords1, "TEXCOORD_1");

            // We don't do Vertex Colours right now
            TArray<FVector3f> Colours;
            GetVertexColours(JsonAttributesObject, Colours);

            UGLTFMaterial* Material = nullptr;
            if (int64 MaterialIndexIndex;
                JsonPrimitiveObject->TryGetNumberField(TEXT("material"), MaterialIndexIndex))
            {
                Material = MaterialIndexToMaterialMap[MaterialIndexIndex];
            }

            UGLTFStaticMeshComponent* GlTFStaticMesh = NewObject<UGLTFStaticMeshComponent>();

            FString PrimitiveSuffix = PrimitiveCount == 0 ? TEXT("") : TEXT("_") + FString::FromInt(PrimitiveCount);


            if (!GlTFStaticMesh->CreateMesh(Name + PrimitiveSuffix,
                                            Vertices,
                                            Indices,
                                            Normals,
                                            TextureCoords0,
                                            Transform,
                                            Material))
            {
                return false;
            }

            GlTFStaticMeshes.Add(GlTFStaticMesh);

            PrimitiveCount++;
        }
    }

    return true;
}

void FGLTFParser::LoadMaterial(TSharedPtr<FJsonObject> JsonMaterial, int32 MaterialIndex)
{
    FGlTFMaterialProperties MaterialProperties{};

    if (!JsonMaterial->TryGetStringField(TEXT("name"), MaterialProperties.Name))
    {
        // When no Material Name 
        MaterialProperties.Name = FString("Material_") + FString::FromInt(MaterialIndex);;
    }

    JsonMaterial->TryGetBoolField(TEXT("doubleSided"), MaterialProperties.bDoubleSided);

    // OPAQUE - No transparency at all
    // BLEND - Uses full alpha blending
    // MASK - Uses alpha cutoff to discard pixels below a threshold
    FString AlphaMode = "OPAQUE";
    JsonMaterial->TryGetStringField(TEXT("alphaMode"), AlphaMode);

    if (AlphaMode == "BLEND")
    {
        MaterialProperties.bTranslucent = true;
    }
    else if (AlphaMode == "MASK")
    {
        MaterialProperties.bMasked = true;
        JsonMaterial->TryGetNumberField(TEXT("alphaCutoff"), MaterialProperties.AlphaCutOff);
    }
    // else if not OPAQUE { Unsupported }

    if (const TSharedPtr<FJsonObject>* JsonPbrObject;
        JsonMaterial->TryGetObjectField(TEXT("pbrMetallicRoughness"), JsonPbrObject))
    {
        if (const TArray<TSharedPtr<FJsonValue>>* JsonBaseColourFactorArray;
            (*JsonPbrObject)->TryGetArrayField(TEXT("baseColorFactor"), JsonBaseColourFactorArray))
        {
            GetJsonVector<4>(JsonBaseColourFactorArray, MaterialProperties.Colour);
        }


        (*JsonPbrObject)->TryGetNumberField(TEXT("roughnessFactor"), MaterialProperties.Roughness);
        (*JsonPbrObject)->TryGetNumberField(TEXT("metallicFactor"), MaterialProperties.Metalness);
    }

    if (const TArray<TSharedPtr<FJsonValue>>* JsonEmissiveFactorArray;
        JsonMaterial->TryGetArrayField(TEXT("emissiveFactor"), JsonEmissiveFactorArray))
    {
        GetJsonVector<3>(JsonEmissiveFactorArray, MaterialProperties.EmissiveColour);
    }

    if (const auto GlTFMaterial = NewObject<UGLTFMaterial>(GetTransientPackage());
        GlTFMaterial->CreateMaterial(MaterialProperties))
    {
        MaterialIndexToMaterialMap.Add(MaterialIndex, GlTFMaterial);
    }
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
    const TArray<TSharedPtr<FJsonValue>>* Materials;
    Root->TryGetArrayField(TEXT("materials"), Materials);
    for (int32 MaterialIndex = 0; MaterialIndex < Materials->Num(); MaterialIndex++)
    {
        TSharedPtr<FJsonObject> JsonMaterialObject = (*Materials)[MaterialIndex]->AsObject();

        if (!JsonMaterialObject)
        {
            continue;
        }

        LoadMaterial(JsonMaterialObject, MaterialIndex);
    }


    // Load All scenes => Usually just 1 with an index of 0
    const TArray<TSharedPtr<FJsonValue>>* JsonScenes;
    if (!Root->TryGetArrayField(TEXT("scenes"), JsonScenes))
    {
        // No Scenes, empty gltf
        return false;
    }

    for (int32 SceneIndex = 0; SceneIndex < JsonScenes->Num(); SceneIndex++)
    {
        const TSharedPtr<FJsonObject> JsonSceneObject = GetJsonObjectFromRootIndex("scenes", SceneIndex);
        if (!JsonSceneObject)
        {
            return false;
        }

        const TArray<TSharedPtr<FJsonValue>>* JsonSceneNodes;
        if (JsonSceneObject->TryGetArrayField(TEXT("nodes"), JsonSceneNodes))
        {
            for (int32 Index = 0; Index < JsonSceneNodes->Num(); Index++)
            {
                const TSharedPtr<FJsonObject> JsonNodeObject = GetJsonObjectFromRootIndex("nodes", Index);

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
