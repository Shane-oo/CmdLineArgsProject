// Fill out your copyright notice in the Description page of Project Settings.


#include "GLTFStaticMeshComponent.h"

#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"


// #region Constructors

UGLTFStaticMeshComponent::UGLTFStaticMeshComponent()
{
}

// #endregion

// #region Public Methods

bool UGLTFStaticMeshComponent::Init(FString Name, TArray<FVector> Vertices, TArray<int32> Indices)
{
    this->Name = Name;

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
    VertexIds.Reserve(Vertices.Num());
    for (const auto Vertex : Vertices)
    {
        VertexIds.Add(MeshDescriptionBuilder.AppendVertex(Vertex));
    }

    // Build Triangles
    FPolygonGroupID PolygonGroup = MeshDescriptionBuilder.AppendPolygonGroup();
    for (int32 i = 0; i < Indices.Num(); i += 3)
    {
        const FVertexInstanceID V0 = MeshDescriptionBuilder.AppendInstance(VertexIds[Indices[i + 0]]);
        const FVertexInstanceID V1 = MeshDescriptionBuilder.AppendInstance(VertexIds[Indices[i + 1]]);
        const FVertexInstanceID V2 = MeshDescriptionBuilder.AppendInstance(VertexIds[Indices[i + 2]]);

        // Set attributes here
        /*
        Attributes.GetVertexInstanceNormals()[V0] = Normals[Indices[i + 0]];
        Attributes.GetVertexInstanceNormals()[V1] = Normals[Indices[i + 1]];
        Attributes.GetVertexInstanceNormals()[V2] = Normals[Indices[i + 2]];

        Attributes.GetVertexInstanceUVs()[V0][0] = UVs[Indices[i + 0]];
        Attributes.GetVertexInstanceUVs()[V1][0] = UVs[Indices[i + 1]];
        Attributes.GetVertexInstanceUVs()[V2][0] = UVs[Indices[i + 2]];
        */

        // Create the triangle from these vertex instances
        MeshDescriptionBuilder.AppendTriangle(V0, V1, V2, PolygonGroup);
    }

    GlTFStaticMesh->BuildFromMeshDescriptions({&MeshDescription});

    SetStaticMesh(GlTFStaticMesh);

    // Add Material at end

    return true;
}

// #endregion
