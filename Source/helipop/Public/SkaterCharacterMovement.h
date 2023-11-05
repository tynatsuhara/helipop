

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SkaterCharacterMovement.generated.h"

/**
 * 
 */
UCLASS()
class HELIPOP_API USkaterCharacterMovementComponent: public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USkaterCharacterMovementComponent();
	~USkaterCharacterMovementComponent();
	virtual bool ShouldRemainVertical() const;
};
