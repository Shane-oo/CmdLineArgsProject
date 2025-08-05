//
// Created by Shane on 5/8/25.
//
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GLTFAsset.generated.h"

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
    //TSharedPtr<FJsonObject> ;
};
