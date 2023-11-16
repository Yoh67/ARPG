// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"

#include "ARPGCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;

struct FEnhancedInputActionValueBinding;

UCLASS()
class ARPG_API AARPGCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	/* Variables */
	FEnhancedInputActionValueBinding* MoveActionBinding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComp;

	/* Functions */
	AARPGCharacterBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE class USpringArmComponent* GetSpringArmComp() const { return SpringArmComp; }
	FORCEINLINE class UCameraComponent* GetCameraComp() const { return CameraComp; }

	//////////////////////////////////////////////////////////////////
	/// Essential Information
	//////////////////////////////////////////////////////////////////

	UPROPERTY(BlueprintReadOnly, Category = "ARPG|Essential Information")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "ARPG|Essential Information")
	FVector2D InputAxisValue = FVector2D::Zero();

	/* Functions */
	UFUNCTION(BlueprintGetter, Category = "ARPG|Essential Information")
	float GetSpeed() const { return Speed; }

protected:

	void SetEssentialValues(float DeltaTime);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//////////////////////////////////////////////////////////////////
	/// Input
	//////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARPG|Enhanced Input")
	class UInputMappingContext* InputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARPG|Enhanced Input")
	class UInputConfigData* InputActions;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
};
