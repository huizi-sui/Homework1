// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyProjectGameMode.generated.h"

class ATargetCube;

DECLARE_MULTICAST_DELEGATE_OneParam(FTotalScoreChangedDelegate, float);
DECLARE_MULTICAST_DELEGATE(FGameEndDelegate);

UCLASS(minimalapi)
class AMyProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyProjectGameMode();

	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditDefaultsOnly)
	int32 TotalCubeCount = 16;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 DoublePointsCubeCount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CubeScore = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ScaleAfterHit = 0.5f;

	int CurrentDoublePointsCubeCount = 0;
	int CurrentCubeCount = 0;
	
	float TotalScore = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float GameTime = 60.0f;

	UPROPERTY(VisibleAnywhere)
	FTimerHandle GameTimerHandle;

	void OnTimerEnd();

public:

	FTotalScoreChangedDelegate OnTotalScoreChanced;
	FGameEndDelegate OnGameEnd;
	
	bool CanSpawnDoublePointsCube() const;
	bool MustSpawnDoublePointsCube() const;
	void AddDoublePointsCubeCount();
	void AddCubeCount();

	FORCEINLINE float GetGameTime() const { return GameTime; }
	FORCEINLINE int32 GetCurrentCubeCount() const { return CurrentCubeCount; }
	FORCEINLINE int32 GetCurrentDoublePointsCubeCount() const { return CurrentDoublePointsCubeCount; }
	FORCEINLINE float GetCubeScore() const { return CubeScore; }
	FORCEINLINE float GetScaleAfterHit() const { return ScaleAfterHit; }
	FORCEINLINE float GetScore() const { return TotalScore; }

	int32 GetDoublePointsCubeCount() const { return DoublePointsCubeCount; }

	void AddScore(float InScore);
};



