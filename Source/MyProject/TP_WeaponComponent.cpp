// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "MyProjectCharacter.h"
#include "MyProjectProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(50.0f, 0.0f, 0.0f);
}

void UTP_WeaponComponent::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr || ProjectileClass == nullptr)
	{
		return;
	}

	Server_Fire();
}

void UTP_WeaponComponent::Server_Fire_Implementation()
{
	// Try and fire a projectile
	if (UWorld* const World = GetWorld())
	{
		const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
		const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = WeaponSkeletalMeshComponent->GetSocketLocation(TEXT("Muzzle")) + SpawnRotation.RotateVector(MuzzleOffset);
		
		const FVector LineEnd = SpawnLocation + SpawnRotation.RotateVector(FVector::ForwardVector * 1000.f);

		UKismetSystemLibrary::DrawDebugLine(World, SpawnLocation, LineEnd, FColor::Red, 10.f, 2.f);

		UKismetSystemLibrary::DrawDebugLine(World, WeaponSkeletalMeshComponent->GetSocketLocation(TEXT("Muzzle")),
			WeaponSkeletalMeshComponent->GetSocketLocation(TEXT("Muzzle")) + SpawnRotation.RotateVector(FVector::ForwardVector * 1000.f),
			FColor::Blue, 10.f, 2.f);

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		// 注册发射子弹玩家
		ActorSpawnParams.Instigator = Character;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// Spawn the projectile at the muzzle
		World->SpawnActor<AMyProjectProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

		Multicast_FireSound();
	}
}

void UTP_WeaponComponent::Multicast_FireSound_Implementation()
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

void UTP_WeaponComponent::AttachWeapon(AMyProjectCharacter* TargetCharacter)
{
	Character = TargetCharacter;
	// Attach the weapon to the First Person Character
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh(), AttachmentRules, FName(TEXT("S_Rifle")));
	
	// switch bHasRifle so the animation blueprint can switch to another animation set
	Character->SetHasRifle(true);
	Client_AddWeaponMappingContext();
}

void UTP_WeaponComponent::Client_AddWeaponMappingContext_Implementation()
{
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
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Fire);
		}
	}
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character == nullptr)
	{
		return;
	}

	if (const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}
