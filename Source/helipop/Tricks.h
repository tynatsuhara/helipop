

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class HELIPOP_API Tricks
{
public:
	Tricks();
	~Tricks();
};

// If you add a new trick, close Unreal and recompile or shit will get wacky
UENUM(BlueprintType)
enum class Trick : uint8
{
	NONE,
	OLLIE,
	GRAB
};