// Copyright Epic Games, Inc. All Rights Reserved.

#include "SwordsRoyaleGameMode.h"
#include "SwordsRoyaleCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASwordsRoyaleGameMode::ASwordsRoyaleGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	UE_LOG(LogTemp, Warning, TEXT("Battle commencing!"));
}
