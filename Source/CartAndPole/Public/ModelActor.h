// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModelActor.generated.h"

class UNneModelComponent;

UCLASS()
class CARTANDPOLE_API AModelActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AModelActor();
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NeuralNetwork", meta = (AllowPrivateAccess = true))
	TArray<float> Inputs;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="NeuralNetwork", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNneModelComponent> ModelComponent;
};
