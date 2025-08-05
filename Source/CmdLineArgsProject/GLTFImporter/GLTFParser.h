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

    static TSharedPtr<FGLTFParser> Create(const FString& FileName);

protected:
    TMap<int32, TObjectPtr<UStaticMesh>> StaticMeshesCache;
};
