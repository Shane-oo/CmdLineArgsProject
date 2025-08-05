// Fill out your copyright notice in the Description page of Project Settings.

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

    void AddReferencedObjects(FReferenceCollector& Collector);
};
