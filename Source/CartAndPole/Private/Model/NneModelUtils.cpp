// Fill out your copyright notice in the Description page of Project Settings.


#include "Model/NneModelUtils.h"
#include "Algo/MaxElement.h"

#include "NNECore.h"
#include "NNECoreModelData.h"
#include "NNECoreRuntimeCPU.h"
#include "NNECoreRuntimeGPU.h"

using namespace UE::NNECore;

bool NneModelUtils::GetRuntimes(TArray<FString>& CpuNames, TArray<FString>& GpuNames)
{
	TArrayView<TWeakInterfacePtr<INNERuntime>> Runtimes = GetAllRuntimes();
    for (const auto& Runtime : Runtimes)
    {
        if (!Runtime.IsValid()) continue;
        if (Cast<INNERuntimeCPU>(Runtime.Get())) CpuNames.Add(Runtime->GetRuntimeName());
        else if (Cast<INNERuntimeGPU>(Runtime.Get())) GpuNames.Add(Runtime->GetRuntimeName());
    }
    return !(CpuNames.IsEmpty() || GpuNames.IsEmpty());
}

int32 NneModelUtils::UnboxOutputValue(const TArray<float>& Outputs)
{
    return Outputs.Find(*Algo::MaxElement(Outputs));
}

bool NneModelUtils::CreateModel(TSharedPtr<IModelCPU>& ModelContainer, const FString& RuntimeName, UNNEModelData* ModelData)
{
    if (!ModelData)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid model data"));
		return false;
	}
 
	TWeakInterfacePtr<INNERuntimeCPU> Runtime = GetRuntime<INNERuntimeCPU>(RuntimeName);
	if (!Runtime.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("No CPU runtime '%s' found"), *RuntimeName);
		return false;
	}
	
	TUniquePtr<IModelCPU> Model = Runtime->CreateModelCPU(ModelData);
	if (!Runtime.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create the CPU model"));
		return false;
	}

	ModelContainer = TSharedPtr<IModelCPU>(Model.Release());
    return ModelContainer.IsValid();
}

bool NneModelUtils::CreateModel(TSharedPtr<IModelGPU>& ModelContainer, const FString& RuntimeName, UNNEModelData* ModelData)
{
    if (!ModelData)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid model data"));
		return false;
	}
 
	TWeakInterfacePtr<INNERuntimeGPU> Runtime = GetRuntime<INNERuntimeGPU>(RuntimeName);
	if (!Runtime.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("No CPU runtime '%s' found"), *RuntimeName);
		return false;
	}
 
	TUniquePtr<IModelGPU> Model = Runtime->CreateModelGPU(ModelData);
	if (!Model.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create the GPU model"));
		return false;
	}

	ModelContainer = TSharedPtr<IModelGPU>(Model.Release());
    return ModelContainer.IsValid();
}
