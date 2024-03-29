// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Skateboard.h"
#include "Tricks.h"
#include "helipopCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AhelipopCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Skating, meta = (AllowPrivateAccess = "true"))
	bool bOnSkateboard;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skating, meta = (AllowPrivateAccess = "true"))
	ASkateboard* Skateboard;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skating, meta = (AllowPrivateAccess = "true"))
	ASkateboard* SkateboardUnderArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Skating, meta = (AllowPrivateAccess = "true"))
	Trick CurrentTrick;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Skating, meta = (AllowPrivateAccess = "true"))
	bool bPushing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Skating, meta = (AllowPrivateAccess = "true"))
	bool bRidingSwitch;

public:
	AhelipopCharacter(const FObjectInitializer& ObjectInitializer);

	void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	void MountSkateboard();

	UFUNCTION(BlueprintCallable)
	void DismountSkateboard();

	UFUNCTION(BlueprintCallable)
	void UpdateRotationSpeed();

	UFUNCTION(BlueprintCallable)
	void HandleCollision(UPrimitiveComponent* MyComp, FVector HitNormal);

	UFUNCTION(BlueprintImplementableEvent)
	void Ragdoll();


protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

