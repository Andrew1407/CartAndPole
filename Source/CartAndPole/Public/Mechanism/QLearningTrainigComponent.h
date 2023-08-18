// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MovementInput.h"
#include "QLearningTrainigComponent.generated.h"

class UUdpClient;
class ACartPoleMechanism;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARTANDPOLE_API UQLearningTrainigComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UQLearningTrainigComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool SocketValid() const;

	UFUNCTION(BlueprintCallable)
	bool CheckHeartbeat();
	
	UFUNCTION(BlueprintCallable)
	bool SendTrainingParams(int32 Episodes, int32 MaxActions);

	UFUNCTION(BlueprintCallable)
	bool ReceiveAction(const TArray<float>& State, int32 Step, int32 Episode, float& ActionRef);

	UFUNCTION(BlueprintCallable)
	bool SendSample(const TArray<float>& State, const TArray<float>& NextState, int32 Action, bool bDone, int32 Step, int32 Episode);

	UFUNCTION(BlueprintCallable)
	bool SaveProgress();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	#pragma region CONFIGURABLE_OPTIONS

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP Connection", meta = (AllowPrivateAccess = true))
	FString Host;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP Connection", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	int32 Port;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UDP Connection", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float ResponseTimeout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UDP Connection", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float MovementFrequency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UDP Connection", meta = (AllowPrivateAccess = true, ClampMin = 1, UIMin = 1))
	int32 TrainingEpisodes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UDP Connection", meta = (AllowPrivateAccess = true, ClampMin = 1, UIMin = 1))
	int32 MaxActionsPerEpisode;

	#pragma endregion

	#pragma region STATE_PARAMS

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 CurrentEpisode;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 EpisodeStep;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 CurrentStep;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float Action;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FMovementInput InputsState;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool IsDone;

	#pragma endregion

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UUdpClient> UdpSocket;

	UPROPERTY()
	TWeakObjectPtr<ACartPoleMechanism> Mechanism;

	#pragma region MOVEMENT_TIMER

	FTimerHandle TimerHandle;

	void TriggerMovement();
	void SetTimer();
	void ClearTimer();

	#pragma endregion

	void ResetEpisode();
	void ShowError(float Duration, const FString& Message) const;
	FString StringifyArray(const TArray<float>& Data) const;

	bool CheckDone() const;
	bool CheckValid() const;

	void ApplyAction();
	void FinishTraining();
};
