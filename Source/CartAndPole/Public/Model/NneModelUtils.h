// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UNNEModelData;

namespace UE
{
    namespace NNECore
    {
        class IModelCPU;
        class IModelGPU;
    }
}

/**
 * 
 */
class CARTANDPOLE_API NneModelUtils
{
public:
    UFUNCTION(BlueprintCallable, Category = "NneModelUtils")
    static bool GetRuntimes(UPARAM(ref) TArray<FString>& CpuNames, UPARAM(ref) TArray<FString>& GpuNames);

    UFUNCTION(BlueprintPure, Category = "NneModelUtils")
    static int32 UnboxOutputValue(const TArray<float>& Outputs);

    static bool CreateModel(TSharedPtr<UE::NNECore::IModelCPU>& ModelContainer, const FString& RuntimeName, UNNEModelData* ModelData);
    static bool CreateModel(TSharedPtr<UE::NNECore::IModelGPU>& ModelContainer, const FString& RuntimeName, UNNEModelData* ModelData);
};
