// Fill out your copyright notice in the Description page of Project Settings.


#include "Mechanism/CartPoleMechanism.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


// Sets default values
ACartPoleMechanism::ACartPoleMechanism(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MaxMovementBound = 810;
	MaxAngleLimit = 45;

	VelocityAcceleration = 15;
	bBlockCartInputMovement = false;
	bCheckMoveActionsLimit = true;

	auto DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRoot);

	SpectatorCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("SpectatorCamera"));
	SpectatorCamera->SetRelativeLocation({ -900, 0, 210 });
	SpectatorCamera->SetupAttachment(RootComponent);

	Cart = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cart"));
	Cart->SetupAttachment(RootComponent);

	Pole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pole"));
	Pole->SetSimulatePhysics(true);
	Pole->SetMassOverrideInKg(NAME_None, 300);
	Pole->SetupAttachment(RootComponent);

	HingeJoint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("HingeJoint"));
	HingeJoint->SetRelativeRotation({ 0, 0, -90 });
	HingeJoint->SetDisableCollision(true);

	HingeJoint->SetLinearXLimit(LCM_Locked, FLT_MAX);
	HingeJoint->SetLinearYLimit(LCM_Locked, FLT_MAX);
	HingeJoint->SetLinearZLimit(LCM_Locked, FLT_MAX);

	HingeJoint->SetAngularSwing1Limit(ACM_Locked, 0);
	HingeJoint->SetAngularSwing2Limit(ACM_Locked, 0);
	SetJointTwistEnabled(true);

	// HingeJoint->ComponentName1.ComponentName = *Cart->GetName();
	// HingeJoint->ComponentName2.ComponentName = *Pole->GetName();

	HingeJoint->SetConstrainedComponents(Cart, NAME_None, Pole, NAME_None);
	HingeJoint->SetupAttachment(RootComponent);
}

void ACartPoleMechanism::PawnClientRestart()
{
	Super::PawnClientRestart();
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller.
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// PawnClientRestart can run more than once in an Actor's lifetime, so start by clearing out any leftover mappings.
			Subsystem->ClearAllMappings();

			// Add each mapping context, along with their priority values. Higher values outprioritize lower values.
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

// Called to bind functionality to input
void ACartPoleMechanism::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &ACartPoleMechanism::Move);
	}
}

void ACartPoleMechanism::SetJointTwistEnabled(bool IsEnabled)
{
	if (IsEnabled)
		HingeJoint->SetAngularTwistLimit(ACM_Limited, MaxAngleLimit);
	else
		HingeJoint->SetAngularTwistLimit(ACM_Locked, 0);
}

void ACartPoleMechanism::SetupInitials()
{
	InitialCartPosition = Cart->GetRelativeLocation().Y;
	InitialPolePosition = Pole->GetRelativeLocation();
	CartLastPosition = InitialCartPosition;
	PoleLastAngle = Pole->GetRelativeRotation().Roll;
	LastUpdate = GetWorld()->GetTimeSeconds();
}

FMovementInput ACartPoleMechanism::GetUpdatedInputs()
{
	float CartPosition = Cart->GetRelativeLocation().Y - InitialCartPosition;
	float PoleAngle = Pole->GetRelativeRotation().Roll;
	double Update = GetWorld()->GetTimeSeconds();
	float Delta = Update - LastUpdate;

	FMovementInput MovementInput;
	MovementInput.CartPosition = CartPosition / MaxMovementBound;
	MovementInput.CartVelocity = (CartPosition - CartLastPosition) / Delta;
	MovementInput.PoleAngle = PoleAngle / MaxAngleLimit;
	MovementInput.PoleAngularVelocity = (PoleAngle - PoleLastAngle) / Delta;

	CartLastPosition = CartPosition;
	PoleLastAngle = PoleAngle;
	LastUpdate = Update;

	return MovementInput;
}

bool ACartPoleMechanism::ShouldReset()
{
	float CartPosition = Cart->GetRelativeLocation().Y - InitialCartPosition;
	float PoleAngle = Pole->GetRelativeRotation().Roll;

	bool IsOutOfBounds = CartPosition >= MaxMovementBound;
	bool IsPoleFallen = FMath::IsNearlyEqual(PoleAngle, MaxAngleLimit, 0.1);
	return IsOutOfBounds || IsPoleFallen;
}

void ACartPoleMechanism::ResetMechanism()
{
	FVector CartLocation = Cart->GetRelativeLocation();
	CartLocation.Y = InitialCartPosition;

	SetJointTwistEnabled(false);
	Cart->SetRelativeLocation(CartLocation);
	Pole->SetRelativeRotation(FRotator::ZeroRotator);
	Pole->SetRelativeLocation(InitialPolePosition);
	SetJointTwistEnabled(true);
}

void ACartPoleMechanism::MoveCart(float Directrion)
{
	Cart->AddRelativeLocation(Cart->GetRightVector() * VelocityAcceleration * Directrion);
}

void ACartPoleMechanism::Move(const FInputActionValue& Value)
{
	if (!bBlockCartInputMovement) MoveCart(Value.Get<float>());
}
