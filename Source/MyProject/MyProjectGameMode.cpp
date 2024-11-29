// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyProjectGameMode.h"
#include "MyProjectCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/ConstructorHelpers.h"

AMyProjectGameMode::AMyProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

void AMyProjectGameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &ThisClass::OnTimerEnd, GameTime, false);
}

void AMyProjectGameMode::OnTimerEnd()
{
	OnGameEnd.Broadcast();
	// UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

bool AMyProjectGameMode::CanSpawnDoublePointsCube() const
{
	return CurrentDoublePointsCubeCount < DoublePointsCubeCount;
}

bool AMyProjectGameMode::MustSpawnDoublePointsCube() const
{
	return DoublePointsCubeCount - CurrentDoublePointsCubeCount >= TotalCubeCount - CurrentCubeCount;
}

void AMyProjectGameMode::AddCubeCount()
{
	CurrentCubeCount++;
}

void AMyProjectGameMode::AddScore(float InScore)
{
	TotalScore += InScore;
	OnTotalScoreChanced.Broadcast(TotalScore);
}

void AMyProjectGameMode::AddDoublePointsCubeCount()
{
	CurrentDoublePointsCubeCount++;
}
