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
};
