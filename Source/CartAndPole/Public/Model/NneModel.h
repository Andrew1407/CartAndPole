// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NneModel.generated.h"

class UNNEModelData;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNneModel : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CARTANDPOLE_API INneModel
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "INneModel")
	bool Predict(const TArray<float>& Inputs, UPARAM(ref) TArray<float>& Outputs);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "INneModel")
	bool ModelDefined();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "INneModel")
	bool TryDefineModel(UNNEModelData* ModelData, const FString& Runtime);
};
