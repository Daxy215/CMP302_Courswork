// Copyright Epic Games, Inc. All Rights Reserved.

#include "CMP302_CourseworkGameMode.h"
#include "CMP302_CourseworkCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACMP302_CourseworkGameMode::ACMP302_CourseworkGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
