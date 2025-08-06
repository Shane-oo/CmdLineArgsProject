// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeActor.h"

#include "CustomStaticMeshComponent.h"
#include "HighResScreenshot.h"

// #region Private Methods

// #endregion

// #region Constructors

ACubeActor::ACubeActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    CustomStaticMeshComponent = CreateDefaultSubobject<UCustomStaticMeshComponent>(TEXT("CustomStaticMeshComponent"));
    CustomStaticMeshComponent->SetupAttachment(Root);
}

// #endregion

// #region Protected Methods

// Called when the game starts or when spawned
void ACubeActor::BeginPlay()
{
    Super::BeginPlay();
}

// #endregion

// #region Public Methods


// Called every frame
void ACubeActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    auto WorldSeconds = GetWorld()->GetTimeSeconds();

    if (PictureWaitStartTime < 0.f)
    {
        PictureWaitStartTime = WorldSeconds;

        return;
    }

    float TimeElapsed = WorldSeconds - PictureWaitStartTime;


    // Wait 10s to take a picture?
    if (!PictureTaken && TimeElapsed >= 5.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("ACubeActor::Tick->Taking Picture!"));

        //FString FileName = TEXT("screenshot.png");


        /*FPaths::ScreenShotDir();
        if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
        {
            //GetHighResScreenshotConfig().FilenameOverride = FileName;
            if (GEngine->GameViewport->Viewport->TakeHighResScreenShot())
            {
                PictureTaken = true;

                const FString FileNameTxt = FString("my_log.txt");
                const auto file = FPaths::LaunchDir() + FileNameTxt;

                FFileHelper::SaveStringToFile(FString("test"),
                                              *file,
                                              FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(),
                                              FILEWRITE_Append);
            }
        }*/

        // shutdown
        //FGenericPlatformMisc::RequestExit(false);
        PictureTaken = true;
    }

    // Wait for the file to save?
    if (PictureTaken && TimeElapsed >= 10.f)
    {
        // shutdown
        //FGenericPlatformMisc::RequestExit(false);
    }
}

void ACubeActor::Init() const
{
    bool UseRedMaterial = false;

    if (const auto ComandLine = FCommandLine::Get(); FParse::Param(ComandLine, TEXT("useRed")))
    {
        UseRedMaterial = true;
    }

    CustomStaticMeshComponent->InitMesh(UseRedMaterial);
}

// #endregion
