// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameInstance.h"

#include "CubeActor.h"
#include "Chaos/PBDRigidClusteringAlgo.h"
#include "CmdLineArgsProject/GLTFImporter/GLTFImporterFunctionLibrary.h"

// #region Protected Methods

void UCustomGameInstance::Init()
{
    Super::Init();

    UE_LOG(LogTemp, Warning, TEXT("UCustomGameInstance::Init"));
}

void UCustomGameInstance::OnStart()
{
    UE_LOG(LogTemp, Warning, TEXT("UCustomGameInstance::OnStart"));


    if (const auto World = GetWorld(); World)
    {
        const auto Position = FVector(605, 0, 92.0);
        const auto Rotation = FRotator::ZeroRotator;
        const FActorSpawnParameters SpawnParameters;

        CubeActor = GetWorld()->SpawnActor<ACubeActor>(ACubeActor::StaticClass(),
                                                       Position,
                                                       Rotation,
                                                       SpawnParameters);

        CubeActor->Init();
    }


    auto glTfAsset = UGLTFImporterFunctionLibrary::ImportGlTF(
        FString("/home/shane/Downloads/c5820f7eccca45a7a659356d42de3ab6.glb")
    );

    if (glTfAsset)
    {
        UE_LOG(LogTemp, Display, TEXT("UCustomGameInstance::glTF Imported"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UCustomGameInstance::OnStart::Error:: Failed To Import GLTF"));
        // shutdown
        //FGenericPlatformMisc::RequestExit(false);
    }
}

void UCustomGameInstance::LoadComplete(const float LoadTime, const FString& MapName)
{
    Super::LoadComplete(LoadTime, MapName);

    UE_LOG(LogTemp, Warning, TEXT("UCustomGameInstance::LoadComplete"));
}

// #endregion
