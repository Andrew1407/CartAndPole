// Fill out your copyright notice in the Description page of Project Settings.


#include "Mechanism/NneModelComponent.h"
#include "NNECoreModelData.h"

#include "Model/NneModelUtils.h"
#include "Model/NneModel.h"
#include "Model/CpuNneModel.h"
#include "Model/GpuNneModel.h"


// Sets default values for this component's properties
UNneModelComponent::UNneModelComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UNneModelComponent::BeginPlay()
{
	Super::BeginPlay();

	CreateModel();
}


// Called every frame
void UNneModelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);	
}

bool UNneModelComponent::Predict(const TArray<float>& Inputs, int32& Result)
{
	TArray<float> Outputs;
	bool bStatus = INneModel::Execute_Predict(Model, Inputs, Outputs);
	Result = NneModelUtils::UnboxOutputValue(Outputs);
	return bStatus;
}

bool UNneModelComponent::CreateModel()
{
	if (!ModelData) return false;

	TArray<FString> CpuRuntimes;
	TArray<FString> GpuRuntimes;
	bool bHasDevices = NneModelUtils::GetRuntimes(CpuRuntimes, GpuRuntimes);
	if (!bHasDevices)
	{
		UE_LOG(LogTemp, Warning, TEXT("No devices found"));
		return false;
	}
	if (bUseGPU)
	{
		if (!GpuRuntimes.IsEmpty())
		{
			Model = NewObject<UGpuNneModel>(this, TEXT("GpuNneModel"));
			return INneModel::Execute_TryDefineModel(Model, ModelData, GpuRuntimes[0]);
		}
		UE_LOG(LogTemp, Warning, TEXT("No GPU devices found"));
	}
	Model = NewObject<UCpuNneModel>(this, TEXT("CpuNneModel"));
	return INneModel::Execute_TryDefineModel(Model, ModelData, CpuRuntimes[0]);
}
