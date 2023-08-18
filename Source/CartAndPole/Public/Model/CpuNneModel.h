// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Model/NneModel.h"
#include "CpuNneModel.generated.h"

class UNNEModelData;

namespace UE
{
    namespace NNECore
    {
        class IModelCPU;
    }
}

/**
 * 
 */
UCLASS()
class CARTANDPOLE_API UCpuNneModel : public UObject, public INneModel
{
	GENERATED_BODY()

public:
	virtual bool Predict_Implementation(const TArray<float>& Inputs, TArray<float>& Outputs);
	virtual bool ModelDefined_Implementation();
	virtual bool TryDefineModel_Implementation(UNNEModelData* ModelData, const FString& Runtime);

private:
	TSharedPtr<UE::NNECore::IModelCPU> Model;
};
