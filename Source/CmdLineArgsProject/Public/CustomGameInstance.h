// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CustomGameInstance.generated.h"

class ACubeActor;
/**
 * 
 */
UCLASS()
class CMDLINEARGSPROJECT_API UCustomGameInstance : public UGameInstance
{
    GENERATED_BODY()

protected:
    virtual void Init() override;

    virtual void OnStart() override;

    virtual void LoadComplete(const float LoadTime, const FString& MapName) override;

private:
    UPROPERTY()
    ACubeActor* CubeActor;
};
