// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "MovementInput.h"
#include "CartPoleMechanism.generated.h"

class UPhysicsConstraintComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputComponent;
class UInputAction;

UCLASS()
class CARTANDPOLE_API ACartPoleMechanism : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACartPoleMechanism(const FObjectInitializer& ObjectInitializer);
	
	// virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void SetupInitials();
	FMovementInput GetUpdatedInputs();
	bool ShouldReset();
	void ResetMechanism();
	void MoveCart(float Direction);

protected:
	// Called when the game starts or when spawned
	virtual void PawnClientRestart() override;

private:
	#pragma region CONFIGURABLE_OPTIONS

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	bool bBlockCartInputMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cart", meta = (AllowPrivateAccess = true))
	bool bCheckMoveActionsLimit;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cart", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float VelocityAcceleration;

	#pragma endregion

	#pragma region IMMUTABLE_OPTIONS

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float MaxAngleLimit;
	
	UPROPERTY(EditDefaultsOnly, Category = "Cart", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float MaxMovementBound;

	#pragma endregion

	#pragma region COMPONENTS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> SpectatorCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MechanismComponents", meta = (AllowPrivateAccess = true))
	TObjectPtr<UPhysicsConstraintComponent> HingeJoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MechanismComponents", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> Cart;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MechanismComponents", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> Pole;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> MoveAction;

	#pragma endregion

	float InitialCartPosition;
	FVector InitialPolePosition;

	float CartLastPosition;
	float PoleLastAngle;
	double LastUpdate;

	void SetJointTwistEnabled(bool IsEnabled);
	void Move(const FInputActionValue& Value);
};
