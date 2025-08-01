// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomMoviePipelineGameMode.h"

// #region Protected Methods

void ACustomMoviePipelineGameMode::BeginPlay()
{
    UE_LOG(LogTemp, Warning, TEXT("ACustomMoviePipelineGameMode::BeginPlay"));

    //Super::BeginPlay();
    if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
    {
        const bool bCinematicMode = true;
        const bool bHidePlayer = true;
        const bool bHideHUD = true;
        const bool bPreventMovement = false;
        const bool bPreventTurning = false;
        PlayerController->SetCinematicMode(bCinematicMode, bHidePlayer, bHideHUD, bPreventMovement, bPreventTurning);


        const auto CommandLine = FCommandLine::Get();
        if (float YawInput; FParse::Value(CommandLine, TEXT("yaw="), YawInput))
        {
            PlayerController->AddYawInput(YawInput);
        }

        if (float PitchInput; FParse::Value(CommandLine, TEXT("pitch="), PitchInput))
        {
            PlayerController->AddPitchInput(PitchInput);
        }
    }
}

// #endregion
