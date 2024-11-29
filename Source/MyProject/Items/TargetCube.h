// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TargetCube.generated.h"

class AMyProjectCharacter;
class UBoxComponent;

UCLASS()
class MYPROJECT_API ATargetCube : public AActor
{
	GENERATED_BODY()
	
public:	
	ATargetCube();
	
protected:

	virtual void BeginPlay() override;

public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* BoxComponent;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* BoxMesh;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float Score = 0.f;

	bool bHit = false;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInstance* DoublePointsCubeMaterial;

	UPROPERTY(ReplicatedUsing = "OnRep_UseDoublePointsCubeMaterial")
	bool bUseDoublePointsCubeMaterial = false;

	UFUNCTION()
	void OnRep_UseDoublePointsCubeMaterial();

public:

	FORCEINLINE bool HasBeenHit() const { return bHit; }
	void Hitting(AMyProjectCharacter* AttackCharacter);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetMeshScale(const FVector& InScale);

};
