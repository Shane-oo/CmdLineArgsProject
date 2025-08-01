// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoviePipelineGameMode.h"
#include "CustomMoviePipelineGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CMDLINEARGSPROJECT_API ACustomMoviePipelineGameMode : public AMoviePipelineGameMode
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
};
