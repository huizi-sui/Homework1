// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetCube.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MyProject/MyProjectCharacter.h"
#include "MyProject/MyProjectGameMode.h"
#include "Net/UnrealNetwork.h"

ATargetCube::ATargetCube()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	SetRootComponent(BoxComponent);

	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	BoxMesh->SetupAttachment(GetRootComponent());
}

void ATargetCube::BeginPlay()
{
	Super::BeginPlay();
	if (AMyProjectGameMode* GameMode = Cast<AMyProjectGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		Score = GameMode->GetCubeScore();
		if (GameMode->MustSpawnDoublePointsCube() || (GameMode->CanSpawnDoublePointsCube() && FMath::RandBool()))
		{
			GameMode->AddDoublePointsCubeCount();
			Score *= 2.f;
			bUseDoublePointsCubeMaterial = true;
			BoxMesh->SetMaterial(0, DoublePointsCubeMaterial);
		}
		GameMode->AddCubeCount();
	}
	else if (bUseDoublePointsCubeMaterial)
	{
		BoxMesh->SetMaterial(0, DoublePointsCubeMaterial);
	}
}

void ATargetCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bUseDoublePointsCubeMaterial);
}

void ATargetCube::OnRep_UseDoublePointsCubeMaterial()
{
	bUseDoublePointsCubeMaterial = true;
	BoxMesh->SetMaterial(0, DoublePointsCubeMaterial);
}

// 服务器端调用该函数
void ATargetCube::Hitting(AMyProjectCharacter* AttackCharacter)
{
	if (!bHit)
	{
		bHit = true;
		const AMyProjectGameMode* GameMode = Cast<AMyProjectGameMode>(UGameplayStatics::GetGameMode(this));
		const float scale = GameMode->GetScaleAfterHit();
		const FVector NewScale(scale);
		Multicast_SetMeshScale(NewScale);
		AttackCharacter->Server_GetScore(Score);
		Score *= 2.f;
	}
	else
	{
		AttackCharacter->Server_GetScore(Score);
		Destroy();
	}
}

void ATargetCube::Multicast_SetMeshScale_Implementation(const FVector& InScale)
{
	BoxComponent->SetWorldScale3D(InScale);
	BoxMesh->SetWorldScale3D(InScale);
}
