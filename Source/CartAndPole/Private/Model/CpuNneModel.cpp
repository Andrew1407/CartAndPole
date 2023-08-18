// Fill out your copyright notice in the Description page of Project Settings.


#include "Model/CpuNneModel.h"
#include "Model/NneModelUtils.h"

#include "NNECore.h"
#include "NNECoreModelData.h"
#include "NNECoreRuntimeCPU.h"

using namespace UE::NNECore;

bool UCpuNneModel::Predict_Implementation(const TArray<float>& Inputs, TArray<float>& Outputs)
{
    if (!INneModel::Execute_ModelDefined(this)) return false;

    auto InputShape = TArray<int32>(Model->GetInputTensorDescs()[0].GetShape().GetData());
	const int8 InputsCount = InputShape[1];
	TArray<float> InputData = Inputs;
	InputData.SetNum(InputsCount);

	TArray<FTensorBindingCPU> InputBindings;
	TArray<FTensorShape> InputShapes;
	InputBindings.SetNum(1);
	InputShapes.SetNum(1);

	InputBindings[0].Data = (void*)InputData.GetData();
	InputBindings[0].SizeInBytes = InputData.Num() * sizeof(float);
	InputShapes[0] = FTensorShape::MakeFromSymbolic(FSymbolicTensorShape::Make(InputShape));

	Model->SetInputTensorShapes(InputShapes);

	auto OutputShape = TArray<int32>(Model->GetOutputTensorDescs()[0].GetShape().GetData());
	const int8 OutputsCount = OutputShape[1];
	
	Outputs.SetNum(OutputsCount);
	TArray<FTensorBindingCPU> OutputBindings;
	OutputBindings.SetNum(1);
	OutputBindings[0].Data = (void*)Outputs.GetData();
	OutputBindings[0].SizeInBytes = Outputs.Num() * sizeof(float);

	int32 Status = Model->RunSync(InputBindings, OutputBindings);
	return Status == 0;
}

bool UCpuNneModel::ModelDefined_Implementation()
{
    return Model.IsValid();
}

bool UCpuNneModel::TryDefineModel_Implementation(UNNEModelData* ModelData, const FString& Runtime)
{
    return NneModelUtils::CreateModel(Model, Runtime, ModelData);
}
