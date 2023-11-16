// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ARPGCharacterBase.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/ARPGPlayerController.h"

#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "InputConfigData.h"
#include "InputActionValue.h"

// Sets default values
AARPGCharacterBase::AARPGCharacterBase()
{
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create Spring Arm to act as a camera boom
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	SpringArmComp->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	SpringArmComp->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	SpringArmComp->bEnableCameraRotationLag = true;
	SpringArmComp->SetupAttachment(RootComponent);

	// We control the rotation of spring arm with pawn control rotation already, this disables a subtle side effect
	// where rotating our CapsuleComponent (eg. caused by bOrientRotationToMovement in Character Movement) 
	// will rotate our spring arm until it self corrects later in the update, this may cause unwanted effects when 
	// using CameraLocation during Tick as it may be slightly offset from our final camera position.
	SpringArmComp->SetUsingAbsoluteRotation(true);

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	CameraComp->SetupAttachment(SpringArmComp);

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 1000.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

// Called when the game starts or when spawned
void AARPGCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context, get the local player subsystem
	if (AARPGPlayerController* PlayerController = Cast<AARPGPlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Clear out existing mapping, and add our mapping
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(InputMapping, 0.0f);
		}
	}
}

// Called every frame
void AARPGCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetEssentialValues(DeltaTime);
}

// Called to bind functionality to input
void AARPGCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Get the EnhancedInputComponent
	if (UEnhancedInputComponent* PEIComp = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind the actions
		PEIComp->BindAction(InputActions->InputMove, ETriggerEvent::Triggered, this, &AARPGCharacterBase::Move);
		PEIComp->BindAction(InputActions->InputLook, ETriggerEvent::Triggered, this, &AARPGCharacterBase::Look);

		PEIComp->BindAction(InputActions->InputJump, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		PEIComp->BindAction(InputActions->InputJump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		MoveActionBinding = &PEIComp->BindActionValue(InputActions->InputMove);
	}
}

void AARPGCharacterBase::SetEssentialValues(float DeltaTime)
{
	InputAxisValue = MoveActionBinding->GetValue().Get<FVector2D>();

	// These values represent how the capsule is moving as well as how it wants to move, and therefore are essential
	// for any data driven animation system. They are also used throughout the system for various functions,
	// so I found it is easiest to manage them all in one place.

	const FVector CurrentVel = GetVelocity();

	// Determine if the character is moving by getting it's speed. The Speed equals the length of the horizontal (x y)
	// velocity, so it does not take vertical movement into account. If the character is moving, update the last
	// velocity rotation. This value is saved because it might be useful to know the last orientation of movement
	// even after the character has stopped.
	Speed = CurrentVel.Size2D();
}

void AARPGCharacterBase::Move(const FInputActionValue& Value)
{
	const FVector2D MoveValue = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Get rotation
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		// Get direction
		const FVector Forward = MovementRotation.RotateVector(FVector::ForwardVector);
		const FVector Lateral = MovementRotation.RotateVector(FVector::RightVector);

		// Apply movement inputs in both axis
		AddMovementInput(Forward, MoveValue.Y);
		AddMovementInput(Lateral, MoveValue.X);
	}
}

void AARPGCharacterBase::Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Add yaw and pitch input to controller
		AddControllerYawInput(LookValue.X);
		AddControllerPitchInput(LookValue.Y);
	}
}