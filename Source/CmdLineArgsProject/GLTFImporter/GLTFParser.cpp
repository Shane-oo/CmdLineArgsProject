//
// Created by Shane on 5/8/25.
//

#include "GLTFParser.h"

// #region Constructors

FGLTFParser::FGLTFParser()
{
}

// #endregion

// #region Destructors

FGLTFParser::~FGLTFParser()
{
}

// #endregion

// #region Public Methods

void FGLTFParser::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObjects(StaticMeshesCache);
    //Collector.AddReferencedObjects(MaterialsCache);
    //Collector.AddReferencedObjects(TexturesCache);
    //Collector.AddReferencedObjects(MaterialsNameCache);
    //Collector.AddReferencedObjects(MetallicRoughnessMaterialsMap);
    //Collector.AddReferencedObjects(SpecularGlossinessMaterialsMap);
}   


TSharedPtr<FGLTFParser> FGLTFParser::Create(const FString& FileName)
{
    if (!FPaths::FileExists(FileName))
    {
        UE_LOG(LogTemp, Error, TEXT("FGLTFParser::Create::Error:: File: %s Does Not Exist"), *FileName);
        return nullptr;
    }

    return nullptr;
}

// #endregion
