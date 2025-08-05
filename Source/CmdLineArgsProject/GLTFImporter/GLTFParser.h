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
    FGLTFParser();

    virtual ~FGLTFParser() override;

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

    static TSharedPtr<FGLTFParser> CreateFromFileName(const FString& FileName);

    static TSharedPtr<FGLTFParser> CreateFromRawData(const uint8* DataPtr, int64 DataNum);

protected:
    TMap<int32, TObjectPtr<UStaticMesh>> StaticMeshesCache;
};
