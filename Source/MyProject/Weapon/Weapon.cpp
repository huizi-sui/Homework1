// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MyProject/MyProjectCharacter.h"
#include "MyProject/MyProjectProjectile.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	MuzzleOffset = FVector(50.0f, 0.0f, 0.0f);

	PickupSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphereComponent"));
	SetRootComponent(PickupSphereComponent);
	PickupSphereComponent->InitSphereRadius(50.f);
	
	WeaponSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkeletalMeshComponent"));
	WeaponSkeletalMeshComponent->SetupAttachment(GetRootComponent());
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	PickupSphereComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnSphereBeginOverlap);
}

void AWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Character = Cast<AMyProjectCharacter>(OtherActor);
	if (Character)
	{
		// 如果角色已经装备武器，则不需要处理
		if (Character->GetHasRifle()) return;
		// Must Set, or the server RPC will not be processed.
		SetOwner(Character);

		// Attach the weapon to the First Person Character
		const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		AttachToComponent(Character->GetMesh(), AttachmentRules, FName(TEXT("S_Rifle")));

		PickupSphereComponent->OnComponentBeginOverlap.RemoveAll(this);
	
		// switch bHasRifle so the animation blueprint can switch to another animation set
		Character->SetHasRifle(true);
		// Set up action bindings
		if (const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
				Subsystem->AddMappingContext(FireMappingContext, 1);
			}

			if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
			{
				// Fire
				EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ThisClass::Fire);
			}
		}
	}
}

void AWeapon::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr || ProjectileClass == nullptr)
	{
		return;
	}

	Server_Fire();
	// const FString Message = FString::Printf(TEXT("Fire, Has Autjproty: %d"), HasAuthority());
	// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, Message);
}

void AWeapon::Server_Fire_Implementation()
{
	// const FString Message = FString::Printf(TEXT("Fire, Has Autjproty: %d"), HasAuthority());
	// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, Message);

	// Try and fire a projectile
	if (UWorld* const World = GetWorld())
	{
		const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
		const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = WeaponSkeletalMeshComponent->GetSocketLocation(TEXT("Muzzle")) + SpawnRotation.RotateVector(MuzzleOffset);
		
		const FVector LineEnd = SpawnLocation + SpawnRotation.RotateVector(FVector::ForwardVector * 1000.f);

		// UKismetSystemLibrary::DrawDebugLine(World, SpawnLocation, LineEnd, FColor::Red, 10.f, 0.5f);

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		// 注册发射子弹玩家
		ActorSpawnParams.Owner = this;
		ActorSpawnParams.Instigator = Character;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// Spawn the projectile at the muzzle
		World->SpawnActor<AMyProjectProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

		Multicast_FireSound();
	}
}

void AWeapon::Multicast_FireSound_Implementation()
{
	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}
	
	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}
