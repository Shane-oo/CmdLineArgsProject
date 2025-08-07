// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CubeActor.generated.h"

class UCustomStaticMeshComponent;

UCLASS()
class CMDLINEARGSPROJECT_API ACubeActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ACubeActor();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    UCustomStaticMeshComponent* CustomStaticMeshComponent;

    UPROPERTY(VisibleAnywhere)
    USceneComponent* Root;

    UPROPERTY()
    UMaterialInstanceDynamic* Material;

    bool PictureTaken = false;
    float PictureWaitStartTime = -1.f;

public:
    void Init() const;

    void AddStaticMeshComponent(UStaticMeshComponent* NewStaticMeshComponent) ;
};
