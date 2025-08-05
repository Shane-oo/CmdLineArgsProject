//
// Created by Shane on 5/8/25.
//

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GLTFImporterFunctionLibrary.generated.h"

class UGLTFAsset;

/*
 * 
 */
UCLASS()
class CMDLINEARGSPROJECT_API UGLTFImporterFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "import glTF scene from FileName"))
    static UGLTFAsset* ImportGlTF(const FString& FileName);
};
