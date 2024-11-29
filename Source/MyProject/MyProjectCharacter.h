// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MyProjectCharacter.generated.h"

class UScoreWidget;
class USpringArmComponent;
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreChangedDelegate, float, Score);

UCLASS(config=Game)
class AMyProjectCharacter : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh3P;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float AimDirection = 0.f;
	
public:
	AMyProjectCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, Replicated)
	bool bHasRifle;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();

	UFUNCTION(Server, Reliable)
	void Server_GetScore(float Score);

	UPROPERTY(BlueprintAssignable)
	FScoreChangedDelegate OnGainScoreChanged;

	UPROPERTY(BlueprintAssignable)
	FScoreChangedDelegate OnTotalScoreChanged;

	UPROPERTY(BlueprintAssignable)
	FScoreChangedDelegate OnGameTimeSet;
	
	FORCEINLINE float GetGainScore() const { return GainScore; }
	FORCEINLINE float GetTotalScore() const { return TotalScore; }

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void CalculateAimDirection();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_CalculateAimDirection();
	
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

private:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing="OnRep_GainScore", meta = (AllowPrivateAccess = "true"))
	float GainScore = 0.f;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing="OnRep_TotalScore", meta = (AllowPrivateAccess = "true"))
	float TotalScore = 0.f;

	UPROPERTY(ReplicatedUsing="OnRep_GameTime")
	float GameTime = 0.f;

	UFUNCTION()
	void OnRep_GainScore();

	UFUNCTION()
	void OnRep_TotalScore();

	UFUNCTION()
	void OnRep_GameTime();

	void TotalScoreChanged(float InTotalScore);

	void OnGameEnd();

	UFUNCTION(Client, Reliable)
	void Client_OnGameEnd();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UScoreWidget> ScoreWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> GameEndWidgetClass;

	UPROPERTY()
	UUserWidget* GameEndWidget;

	UPROPERTY()
	UScoreWidget* ScoreWidget;
};

