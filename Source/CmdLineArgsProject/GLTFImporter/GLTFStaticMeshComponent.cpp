// Fill out your copyright notice in the Description page of Project Settings.


#include "GLTFStaticMeshComponent.h"

#include "GLTFMaterial.h"
#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"

// #region Private Methods

void UGLTFStaticMeshComponent::CreateStaticMeshFromPrimitives(const FString& Name,
                                                              const TArray<FVector>& Vertices,
                                                              const TArray<int32>& Indices,
                                                              const TArray<FVector3f>& Normals,
                                                              const TArray<FVector2f>& TextureCoords0,
                                                              const TArray<FVector2f>& TextureCoords1)
{
    ComputedStaticMesh = NewObject<UStaticMesh>(this, *FString::Printf(TEXT("SM_%s"), *Name));

    FMeshDescription MeshDescription;

    FStaticMeshAttributes Attributes(MeshDescription);
    Attributes.Register();

    FMeshDescriptionBuilder MeshDescriptionBuilder;
    MeshDescriptionBuilder.SetMeshDescription(&MeshDescription);
    MeshDescriptionBuilder.EnablePolyGroups();
    MeshDescriptionBuilder.SetNumUVLayers(2);

    // Create Vertices
    TArray<FVertexID> VertexIds;
    TArray<FVertexInstanceID> VertexInstanceIds; // One instance per vertex

    VertexIds.Reserve(Vertices.Num());
    VertexInstanceIds.Reserve(Vertices.Num());

    const auto VertexInstanceNormals = Attributes.GetVertexInstanceNormals();
    const auto VertexInstanceUVs = Attributes.GetVertexInstanceUVs();
    VertexInstanceUVs.SetNumChannels(2);

    for (int32 i = 0; i < Vertices.Num(); ++i)
    {
        const FVertexID VertexID = MeshDescriptionBuilder.AppendVertex(Vertices[i]);
        VertexIds.Add(VertexID);

        const FVertexInstanceID InstanceID = MeshDescriptionBuilder.AppendInstance(VertexID);
        VertexInstanceIds.Add(InstanceID);

        if (!Normals.IsEmpty())
        {
            VertexInstanceNormals.Set(InstanceID, Normals[i]);
        }

        if (!TextureCoords0.IsEmpty())
        {
            VertexInstanceUVs.Set(InstanceID, 0, TextureCoords0[i]);
        }

        if (!TextureCoords1.IsEmpty())
        {
            VertexInstanceUVs.Set(InstanceID, 1, TextureCoords1[i]);
        }
    }

    // Build Triangles
    const FPolygonGroupID PolygonGroup = MeshDescriptionBuilder.AppendPolygonGroup();
    for (int32 i = 0; i < Indices.Num(); i += 3)
    {
        const FVertexInstanceID V0 = VertexInstanceIds[Indices[i + 0]];
        const FVertexInstanceID V1 = VertexInstanceIds[Indices[i + 1]];
        const FVertexInstanceID V2 = VertexInstanceIds[Indices[i + 2]];

        //Unreal uses counter-clockwise (CCW) winding order
        MeshDescriptionBuilder.AppendTriangle(V0, V2, V1, PolygonGroup);
    }

    UStaticMesh::FBuildMeshDescriptionsParams DescriptionsParams;
    DescriptionsParams.bFastBuild = true;
    DescriptionsParams.bAllowCpuAccess = true;

    ComputedStaticMesh->BuildFromMeshDescriptions({&MeshDescription}, DescriptionsParams);

    ComputedStaticMesh->bAllowCPUAccess = true;

    for (FStaticMeshLODResources& LOD : ComputedStaticMesh->GetRenderData()->LODResources)
    {
        for (FStaticMeshSection& Section : LOD.Sections)
        {
            // Force to first material slot
            Section.MaterialIndex = 0;
        }
    }

    // Rebuild GPU data
    ComputedStaticMesh->InitResources();

    SetStaticMesh(ComputedStaticMesh);
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
                                          const TArray<FVector2f>& TextureCoords1,
                                          const FTransform& Transform,
                                          const UGLTFMaterial* GlTFMaterial)
{
    this->Name = Name;

    SetRelativeTransform(Transform);

    CreateStaticMeshFromPrimitives(Name, Vertices, Indices, Normals, TextureCoords0, TextureCoords1);

    if (GlTFMaterial)
    {
        ComputedMaterial = GlTFMaterial->GetMaterial();
        SetMaterial(0, ComputedMaterial);
    }


    return true;
}

// #endregion
