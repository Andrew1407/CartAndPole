// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CartAndPoleGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class CARTANDPOLE_API ACartAndPoleGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
