//
// Created by Shane on 5/8/25.
//
#pragma once

#include "CoreMinimal.h"
#include "GLTFAsset.generated.h"

class FGLTFParser;
/**
 * 
 */
UCLASS()
class CMDLINEARGSPROJECT_API UGLTFAsset : public UObject
{
    GENERATED_BODY()

public:
    bool LoadFromFileName(const FString& FileName);

protected:
    TSharedPtr<FGLTFParser> Parser;
};
