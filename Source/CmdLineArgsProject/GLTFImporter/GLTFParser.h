//
// Created by Shane on 5/8/25.
//

#pragma once

#include "CoreMinimal.h"

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

    TMap<int32, TObjectPtr<UStaticMesh>> StaticMeshesCache;

public:
    virtual FString GetReferencerName() const override
    {
        return TEXT("FGLTFParser");
    }

    void SetBinaryBuffer(const TArray64<uint8>& BinaryBuffer)
    {
        this->BinaryBuffer = BinaryBuffer;
    }

    TArray<TObjectPtr<UStaticMesh>> GetStaticMeshes()
    {
        TArray<TObjectPtr<UStaticMesh>> Result;
        Result.Reserve(StaticMeshesCache.Num());

        for (const TPair<int32, TObjectPtr<UStaticMesh>>& Pair : StaticMeshesCache)
        {
            Result.Add(Pair.Value);
        }

        return Result;
    }

private:
    void CheckExtensionsRequired() const;

    static bool CheckJsonIndex(const TSharedPtr<FJsonObject>& JsonObject,
                               const FString& FieldName,
                               const int32 Index,
                               TArray<TSharedRef<FJsonValue>>& JsonItems);

    static TSharedPtr<FJsonObject> GetJsonObjectFromIndex(const TSharedPtr<FJsonObject>& JsonObject,
                                                          const FString& FieldName,
                                                          const int32 Index);

    TSharedPtr<FJsonObject> GetJsonObjectFromRootIndex(const FString& FieldName, const int32 Index) const;

    static FString GetJsonObjectString(const TSharedPtr<FJsonObject>& JsonObject,
                                       const FString& FieldName,
                                       const FString& DefaultValue);

    template <int32 Num, typename T>
    static bool GetJsonVector(const TArray<TSharedPtr<FJsonValue>>* JsonValues, T& Value);

    static bool FillJsonMatrix(const TArray<TSharedPtr<FJsonValue>>* JsonValues, FMatrix& Matrix);

    bool LoadNode(TSharedPtr<FJsonObject> JsonNode, int32 NodeIndex);
};
