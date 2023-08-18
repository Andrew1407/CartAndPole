// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NneModelComponent.generated.h"

class UNNEModelData;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARTANDPOLE_API UNneModelComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNneModelComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	bool Predict(const TArray<float>& Inputs, UPARAM(ref) int32& Result);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeuralNetwork", meta = (AllowPrivateAccess = true))
	bool bUseGPU = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeuralNetwork", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNNEModelData> ModelData;

	UPROPERTY(BlueprintReadOnly, Category = "NeuralNetwork", meta = (AllowPrivateAccess = true))
	TObjectPtr<UObject> Model;

	bool CreateModel();
};
