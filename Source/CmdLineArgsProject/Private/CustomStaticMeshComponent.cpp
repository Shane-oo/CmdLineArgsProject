// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomStaticMeshComponent.h"

#include "DynamicMesh/MeshTransforms.h"

// #region Constructors

UCustomStaticMeshComponent::UCustomStaticMeshComponent()
{
}

// #endregion

// #region Public Methods

void UCustomStaticMeshComponent::InitMesh(const bool UseRedMaterial)
{
    const auto MaterialLocation = UseRedMaterial
                                      ? TEXT("/Game/RedMaterial.RedMaterial")
                                      : TEXT("/Game/BlueMaterial.BlueMaterial");
    Material = LoadObject<UMaterialInterface>(nullptr, MaterialLocation);

    if (Material)
    {
        //SetMaterial(0, Material);
    }

    CubeMesh = LoadObject<UStaticMesh>(
        nullptr, TEXT("/Game/Low_Poly_Viking_Interiors/Models/Props/SM_Skull_Deer_01.SM_Skull_Deer_01"));
    if (CubeMesh)
    {
        //SetStaticMesh(CubeMesh);
    }

    float ScaleX;
    float ScaleY;
    float ScaleZ;

    const auto CommandLine = FCommandLine::Get();

    if (FParse::Value(CommandLine, TEXT("scaleX="), ScaleX)
        && FParse::Value(CommandLine, TEXT("scaleY="), ScaleY)
        && FParse::Value(CommandLine, TEXT("scaleZ="), ScaleZ))
    {
        FTransform Transform;
        Transform.SetScale3D(FVector(ScaleX, ScaleY, ScaleZ));
        SetRelativeTransform(Transform);
    }
}

// #endregion
