// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyProjectCharacter.h"
#include "MyProjectProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MyProjectGameMode.h"
#include "MyProjectPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Items/TargetCube.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/ScoreWidget.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMyProjectCharacter

AMyProjectCharacter::AMyProjectCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh3P->SetupAttachment(GetMesh());
	Mesh3P->SetHiddenInGame(true);
	Mesh3P->bCastHiddenShadow = true;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());
	// SpringArm->SetupAttachment(GetMesh(), TEXT("S_Camera"));
	
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->TargetArmLength = 0.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	// Create a CameraComponent	
	// FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	// FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	// FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	// FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	// Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	// Mesh1P->SetOnlyOwnerSee(true);
	// Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	// Mesh1P->bCastDynamicShadow = false;
	// Mesh1P->CastShadow = false;
	// Mesh1P->bCastHiddenShadow = true;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	// Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

void AMyProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("S_Camera"));

	// Add Input Mapping Context
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		GetMesh()->HideBoneByName(TEXT("head"), PBO_None);
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			if (ScoreWidget == nullptr)
			{
				ScoreWidget = CreateWidget<UScoreWidget>(GetWorld(), ScoreWidgetClass);
				ScoreWidget->AddToViewport();
			}
		}
	}

	if (AMyProjectGameMode* GameMode = Cast<AMyProjectGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->OnTotalScoreChanced.AddUObject(this, &ThisClass::TotalScoreChanged);
		GameMode->OnGameEnd.AddUObject(this, &ThisClass::OnGameEnd);
		GameTime = GameMode->GetGameTime();
		OnGameTimeSet.Broadcast(GameTime);
	}
}

//////////////////////////////////////////////////////////////////////////// Input

void AMyProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyProjectCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyProjectCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMyProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AMyProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
		CalculateAimDirection();
	}
}

void AMyProjectCharacter::CalculateAimDirection()
{
	if (HasAuthority())
	{
		const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation());
		AimDirection = DeltaRotator.Pitch;
	}
	else
	{
		Server_CalculateAimDirection();
	}
}

void AMyProjectCharacter::Server_CalculateAimDirection_Implementation()
{
	CalculateAimDirection();
}

void AMyProjectCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AMyProjectCharacter::GetHasRifle()
{
	return bHasRifle;
}

void AMyProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AimDirection);
	DOREPLIFETIME(ThisClass, bHasRifle);
	DOREPLIFETIME(ThisClass, GainScore);
	DOREPLIFETIME(ThisClass, TotalScore);
	DOREPLIFETIME(ThisClass, GameTime);
}

void AMyProjectCharacter::Server_GetScore_Implementation(float Score)
{
	GainScore += Score;
	OnGainScoreChanged.Broadcast(GainScore);
	AMyProjectGameMode* GameMode = Cast<AMyProjectGameMode>(UGameplayStatics::GetGameMode(this));
	GameMode->AddScore(Score);

	// FString Message = FString::Printf(TEXT("My Score: %f, Total Score: %f"), GainScore, TotalScore);
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, Message);
}

void AMyProjectCharacter::OnRep_TotalScore()
{
	OnTotalScoreChanged.Broadcast(TotalScore);
}

void AMyProjectCharacter::OnRep_GameTime()
{
	OnGameTimeSet.Broadcast(GameTime);
}

void AMyProjectCharacter::TotalScoreChanged(float InTotalScore)
{
	TotalScore = InTotalScore;
	OnTotalScoreChanged.Broadcast(TotalScore);
}

void AMyProjectCharacter::OnGameEnd()
{
	Client_OnGameEnd();
}

void AMyProjectCharacter::Client_OnGameEnd_Implementation()
{
	ScoreWidget->RemoveFromParent();
	GameEndWidget = CreateWidget(GetWorld(), GameEndWidgetClass);
	GameEndWidget->AddToViewport();
}

void AMyProjectCharacter::OnRep_GainScore()
{
	OnGainScoreChanged.Broadcast(GainScore);
}
