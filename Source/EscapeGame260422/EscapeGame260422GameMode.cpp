// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeGame260422GameMode.h"
#include "EscapeGame260422Character.h"
#include "UObject/ConstructorHelpers.h"

AEscapeGame260422GameMode::AEscapeGame260422GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
