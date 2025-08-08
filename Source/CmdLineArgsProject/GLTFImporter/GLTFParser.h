//
// Created by Shane on 5/8/25.
//

#pragma once

#include "CoreMinimal.h"
#include "GLTFStaticMeshComponent.h"

class UGLTFAsset;
/**
 * 
 */
class CMDLINEARGSPROJECT_API FGLTFParser : public FGCObject, public TSharedFromThis<FGLTFParser>
{
public:
    explicit FGLTFParser(const TSharedPtr<FJsonObject>& JsonObject);

    virtual ~FGLTFParser() override;

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

    static TSharedPtr<FGLTFParser> CreateFromFileName(const FString& FileName);

    static TSharedPtr<FGLTFParser> CreateFromRawData(const uint8* DataPtr, int64 DataNum);

    static TSharedPtr<FGLTFParser> CreateFromBinaryData(const uint8* DataPtr, int64 DataNum);

    static TSharedPtr<FGLTFParser> CreateFromString(const FString& GlTFJsonData);

    bool LoadScene();

protected:
    TArray64<uint8> BinaryBuffer;

    TSharedPtr<FJsonObject> Root;

    TMap<int32, TObjectPtr<UGLTFStaticMeshComponent>> StaticMeshesCache;

    TArray<UGLTFStaticMeshComponent*> GlTFStaticMeshes;

public:
    virtual FString GetReferencerName() const override
    {
        return TEXT("FGLTFParser");
    }

    void SetBinaryBuffer(const TArray64<uint8>& BinaryBuffer)
    {
        this->BinaryBuffer = BinaryBuffer;
    }

    TArray<TObjectPtr<UGLTFStaticMeshComponent>> GetStaticMeshesOLD()
    {
        TArray<TObjectPtr<UGLTFStaticMeshComponent>> Result;
        Result.Reserve(StaticMeshesCache.Num());

        for (const TPair<int32, TObjectPtr<UGLTFStaticMeshComponent>>& Pair : StaticMeshesCache)
        {
            Result.Add(Pair.Value);
        }

        return Result;
    }

    TArray<UGLTFStaticMeshComponent*> GetStaticMeshes()
    {
        return GlTFStaticMeshes;
    }

private:
    struct FBuffer
    {
        uint8* Data;
        int64 Num;
        int64 ByteOffset;

        FBuffer()
        {
            Data = nullptr;
            Num = 0;
            ByteOffset = 0;
        }
    };

    struct FGlTMaterialProperties
    {
        FString Name;
        bool bDoubleSided = false;

        // OPAQUE - No transparency at all
        // BLEND - Uses full alpha blending
        // MASK - Uses alpha cutoff to discard pixels below a threshold
        FString AlphaMode = "OPAQUE";
        bool bTranslucent = false;
        bool bMasked = false;
        float AlphaCutOff = 0.5f;
    };

    enum EGlTFComponentType
    {
        Gltf_Byte = 5120,
        Gltf_Unsigned_Byte = 5121,
        Gltf_Short = 5122,
        Gltf_Unsigned_Short = 5123,
        Gltf_INT = 5124,
        Gltf_Unsigned_INT = 5125,
        Gltf_Float = 5126
    };

    void CheckExtensionsRequired() const;

    static bool CheckJsonIndex(const TSharedPtr<FJsonObject>& JsonObject,
                               const FString& FieldName,
                               const int32 Index,
                               TArray<TSharedRef<FJsonValue>>& JsonItems);

    static TSharedPtr<FJsonObject> GetJsonObjectFromIndex(const TSharedPtr<FJsonObject>& JsonObject,
                                                          const FString& FieldName,
                                                          const int32 Index);

    TSharedPtr<FJsonObject> GetJsonObjectFromRootIndex(const FString& FieldName, const int32 Index) const;

    static int64 GetJsonObjectIndex(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName);

    static FString GetJsonObjectString(const TSharedPtr<FJsonObject>& JsonObject,
                                       const FString& FieldName,
                                       const FString& DefaultValue);

    template <int32 Num, typename T>
    static bool GetJsonVector(const TArray<TSharedPtr<FJsonValue>>* JsonValues, T& Value);

    static bool FillJsonMatrix(const TArray<TSharedPtr<FJsonValue>>* JsonValues, FMatrix& Matrix);

    bool GetBufferView(const int32 BufferViewIndex, FBuffer& OutBuffer);

    bool GetBuffer(const int32 BufferIndex, FBuffer& OutBuffer);

    bool LoadNode(TSharedPtr<FJsonObject> JsonNode, int32 NodeIndex);

    void LoadMaterial(TSharedPtr<FJsonObject> JsonMaterial, int32 MaterialIndex);

    bool GetVertices(const TSharedPtr<FJsonObject>* JsonAttributesObject, TArray<FVector>& Vertices);

    void GetIndices(const TSharedPtr<FJsonObject>* JsonPrimitiveObject, TArray<int32>& Indices);

    void GetNormals(const TSharedPtr<FJsonObject>* JsonAttributesObject, TArray<FVector3f>& Normals);

    void GetTangents(const TSharedPtr<FJsonObject>* JsonAttributesObject, TArray<FVector4f>& Tangents);

    // TextureCoordField = TEXCOORD_0 OR TEXCOORD_1
    void GetTextureCoords(const TSharedPtr<FJsonObject>* JsonAttributesObject, TArray<FVector2f>& TextureCoords,
                          const FString& TextureCoordField);

    void GetVertexColours(const TSharedPtr<FJsonObject>* JsonAttributesObject, TArray<FVector3f>& Colours);
};
