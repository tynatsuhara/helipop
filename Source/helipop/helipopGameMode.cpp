// Copyright Epic Games, Inc. All Rights Reserved.

#include "helipopGameMode.h"
#include "helipopCharacter.h"
#include "UObject/ConstructorHelpers.h"

AhelipopGameMode::AhelipopGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
