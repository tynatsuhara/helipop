// Copyright Epic Games, Inc. All Rights Reserved.

#include "helipopCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AhelipopCharacter

AhelipopCharacter::AhelipopCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void AhelipopCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AhelipopCharacter::MountSkateboard()
{
	if (!bOnSkateboard) {
		SkateboardUnderArm->SetActorHiddenInGame(true);
		Skateboard->SetActorHiddenInGame(false);

		FTransform ft = GetMesh()->GetRelativeTransform();
		GetMesh()->SetRelativeLocation(ft.TransformPosition(FVector{ 0, 0, 14 }));

		GetCharacterMovement()->BrakingDecelerationWalking = 250;
		GetCharacterMovement()->BrakingDecelerationFalling = 200;
		GetCharacterMovement()->MaxWalkSpeed = 5;

		bOnSkateboard = true;
	}
}

void AhelipopCharacter::DismountSkateboard()
{
	if (bOnSkateboard) {
		CurrentTrick = Trick::NONE;

		SkateboardUnderArm->SetActorHiddenInGame(false);
		Skateboard->SetActorHiddenInGame(true);

		FTransform ft = GetMesh()->GetRelativeTransform();
		GetMesh()->SetRelativeLocation(ft.TransformPosition(FVector{ 0, 0, -14 }));

		GetCharacterMovement()->BrakingDecelerationWalking = 2000;
		GetCharacterMovement()->BrakingDecelerationFalling = 1500;
		GetCharacterMovement()->MaxWalkSpeed = 500;

		bOnSkateboard = false;
	}
}

void AhelipopCharacter::UpdateRotationSpeed()
{
	double rotSpeed = 500;

	if (bOnSkateboard) {
		if (GetCharacterMovement()->IsFalling()) {
			rotSpeed = 600;
		}
		else {
			rotSpeed = 200;
		}
	}

	GetCharacterMovement()->RotationRate = FRotator{ 0, rotSpeed, 0 };
}

void AhelipopCharacter::HandleCollision(UPrimitiveComponent* MyComp, FVector HitNormal)
{
	if (!bOnSkateboard) {
		return;
	}

	// We don't really care about z velocity because you can land (assuming you're landing upright)
	auto v = MyComp->GetComponentVelocity();
	v.Z = 0;
	const auto xySpeed = v.Length();

	const auto up = GetActorUpVector();
	const auto upDot = acos(up.Dot(HitNormal)) * 180/PI;

	const int uprightAngleTolerance = 10;
	const bool landing = upDot >= -uprightAngleTolerance && upDot <= uprightAngleTolerance;

	if (landing) {
		// I don't really understand why we have 2 different velocities which behave differently
		auto velocity = GetVelocity();
		velocity.Normalize();
		const auto dot = velocity.Dot(GetActorForwardVector());
		const int landingAngleTolerance = 35;
		const auto landingAngle = acos(dot) * 180 / PI;
		UE_LOG(LogTemp, Warning, TEXT("%f"), landingAngle);

		const bool shouldRagdollBasedOnSpeed = xySpeed > 700;
		const bool landingForward = isnan(landingAngle) || landingAngle < landingAngleTolerance;
		const bool landingOpposite = landingAngle > 180 - landingAngleTolerance;
		if (shouldRagdollBasedOnSpeed && !landingForward && !landingOpposite) {
			Ragdoll();
		}
		else if (landingOpposite) {
			bRidingSwitch = !bRidingSwitch;
			GetMesh()->SetRelativeScale3D(FVector(bRidingSwitch ? -1 : 1, 1, 1));
			AddActorLocalRotation(FRotator(0, 180, 0));
		}
	}
	else {
		const bool shouldRagdollBasedOnSpeed = xySpeed > 1000;
		if (shouldRagdollBasedOnSpeed) {
			Ragdoll();
		}
	}
}

void AhelipopCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AhelipopCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AhelipopCharacter::Move);
		// Call again once "completed" to reset vars
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AhelipopCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AhelipopCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AhelipopCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if (bOnSkateboard) {
			// skating velocity comes from the animation pushing hook, this block is just used to orient them in the right direction
			const float scale = 0.01;

			// we only allow skating forward
			if (MovementVector.Y > 0) {
				AddMovementInput(GetActorForwardVector(), MovementVector.Y * scale);
			}

			AddMovementInput(GetActorRightVector(), MovementVector.X * scale);

			bPushing = MovementVector.Y > 0;
		}
		else {
			// forward walks in the direction the camera is facing
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			// get right vector 
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// add movement 
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void AhelipopCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}