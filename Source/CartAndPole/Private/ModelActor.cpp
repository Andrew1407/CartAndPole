// Fill out your copyright notice in the Description page of Project Settings.


#include "ModelActor.h"
#include "Mechanism/NneModelComponent.h"
#include "Async/Async.h"

// Sets default values
AModelActor::AModelActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ModelComponent = CreateDefaultSubobject<UNneModelComponent>(TEXT("neModelComponent"));
}

// Called when the game starts or when spawned
void AModelActor::BeginPlay()
{
	Super::BeginPlay();
	
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		int32 Result;
		bool bStatus = ModelComponent->Predict(Inputs, Result);

		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Emerald, FString::Printf(TEXT("bStatus: %d, Result: %d"), bStatus, Result));
	});
}

// Called every frame
void AModelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

