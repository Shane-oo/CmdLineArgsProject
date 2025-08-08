// Fill out your copyright notice in the Description page of Project Settings.


#include "GLTFStaticMeshComponent.h"

#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"

// #region Private Methods

void UGLTFStaticMeshComponent::CreateStaticMeshFromPrimitives(const FString& Name,
                                                              TArray<FVector> Vertices,
                                                              TArray<int32> Indices,
                                                              TArray<FVector3f> Normals,
                                                              TArray<FVector2f> TextureCoords0)
{
    GlTFStaticMesh = NewObject<UStaticMesh>(this, *FString::Printf(TEXT("SM_%s"), *Name));

    FMeshDescription MeshDescription;
    FStaticMeshAttributes Attributes(MeshDescription);
    Attributes.Register();

    FMeshDescriptionBuilder MeshDescriptionBuilder;
    MeshDescriptionBuilder.SetMeshDescription(&MeshDescription);
    MeshDescriptionBuilder.EnablePolyGroups();
    MeshDescriptionBuilder.SetNumUVLayers(1); // for now just 1 

    // Create Vertices
    TArray<FVertexID> VertexIds;
    TArray<FVertexInstanceID> VertexInstanceIds; // One instance per vertex

    VertexIds.Reserve(Vertices.Num());
    VertexInstanceIds.Reserve(Vertices.Num());

    for (int32 i = 0; i < Vertices.Num(); ++i)
    {
        const FVertexID VertexID = MeshDescriptionBuilder.AppendVertex(Vertices[i]);
        VertexIds.Add(VertexID);

        const FVertexInstanceID InstanceID = MeshDescriptionBuilder.AppendInstance(VertexID);
        VertexInstanceIds.Add(InstanceID);

        if (!Normals.IsEmpty())
        {
            Attributes.GetVertexInstanceNormals()[InstanceID] = Normals[i];
        }

        if (!TextureCoords0.IsEmpty())
        {
            Attributes.GetVertexInstanceUVs()[InstanceID] = TextureCoords0[i];
        }
    }

    // Build Triangles
    const FPolygonGroupID PolygonGroup = MeshDescriptionBuilder.AppendPolygonGroup();
    for (int32 i = 0; i < Indices.Num(); i += 3)
    {
        const FVertexInstanceID V0 = MeshDescriptionBuilder.AppendInstance(VertexIds[Indices[i + 0]]);
        const FVertexInstanceID V1 = MeshDescriptionBuilder.AppendInstance(VertexIds[Indices[i + 1]]);
        const FVertexInstanceID V2 = MeshDescriptionBuilder.AppendInstance(VertexIds[Indices[i + 2]]);

        //Unreal uses counter-clockwise (CCW) winding order
        MeshDescriptionBuilder.AppendTriangle(V0, V2, V1, PolygonGroup);
    }

    GlTFStaticMesh->BuildFromMeshDescriptions({&MeshDescription});

    SetStaticMesh(GlTFStaticMesh);
}

// #endregion

// #region Constructors

UGLTFStaticMeshComponent::UGLTFStaticMeshComponent()
{
}

// #endregion

// #region Public Methods


bool UGLTFStaticMeshComponent::CreateMesh(FString Name,
                                          const TArray<FVector>& Vertices,
                                          const TArray<int32>& Indices,
                                          const TArray<FVector3f>& Normals,
                                          const TArray<FVector2f>& TextureCoords0,
                                          const FTransform& Transform)
{
    this->Name = Name;
    SetRelativeTransform(Transform);

    CreateStaticMeshFromPrimitives(Name, Vertices, Indices, Normals, TextureCoords0);

    // SetMaterial

    return true;
}

// #endregion
