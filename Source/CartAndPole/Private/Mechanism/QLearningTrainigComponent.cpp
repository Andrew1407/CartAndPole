// Fill out your copyright notice in the Description page of Project Settings.


#include "Mechanism/QLearningTrainigComponent.h"
#include "Mechanism/CartPoleMechanism.h"
#include "Connection/UdpClient.h"
#include "Async/Async.h"

// Sets default values for this component's properties
UQLearningTrainigComponent::UQLearningTrainigComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Host = "127.0.0.1";
	Port = 8080;

	MovementFrequency = .3f;
	ResponseTimeout = 3;

	TrainingEpisodes = 300;
	MaxActionsPerEpisode = 300;

	CurrentEpisode = 0;
	CurrentStep = 0;
	EpisodeStep = 0;

	UdpSocket = CreateDefaultSubobject<UUdpClient>(TEXT("UdpClient"));
}

void UQLearningTrainigComponent::BeginPlay()
{
	Super::BeginPlay();

	Mechanism = Cast<ACartPoleMechanism>(GetOwner());
	if (!Mechanism.IsValid()) return;
	Mechanism->SetupInitials();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		UdpSocket->CreateConnection(Host, Port);
		if (CheckValid()) return;
		if (!CheckHeartbeat())
			return ShowError(5, TEXT("Heartbeat check failed"));
		if (!SendTrainingParams(TrainingEpisodes, MaxActionsPerEpisode))
			return ShowError(5, TEXT("Failed to set training params"));
		if (!ReceiveAction(InputsState.ToArray(), CurrentStep, EpisodeStep, Action))
			return ShowError(5, TEXT("Action not received"));
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			if (CheckValid()) return;
			CurrentEpisode = 0;
			InputsState = Mechanism->GetUpdatedInputs();
			Mechanism->MoveCart(Action);
			SetTimer();
			IsDone = false;
		});
	});
}

void UQLearningTrainigComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsDone || !Mechanism.IsValid()) return;
	if (CheckDone()) IsDone = true;
}

bool UQLearningTrainigComponent::SocketValid() const
{
	return UdpSocket && UdpSocket->SocketValid();
}

bool UQLearningTrainigComponent::CheckHeartbeat()
{
	if (!SocketValid()) return false;
	const FString Heartbeat = TEXT("heartbeat");
	bool IsSent = UdpSocket->SendData(Heartbeat);
	if (!IsSent) return false;
	const uint32 BufferSize = 1 << 12;
	const FString Response = UdpSocket->ReceiveData(ResponseTimeout, BufferSize);
	return Response.ToLower() == (Heartbeat + TEXT(": ok"));
}

bool UQLearningTrainigComponent::SendTrainingParams(int32 Episodes, int32 MaxActions)
{
	if (!SocketValid()) return false;
	const FString Title = TEXT("training_params: ");
	const FString DataToSend = FString::Printf(TEXT("%s%d %d"), *Title, Episodes, MaxActions);
	bool IsSent = UdpSocket->SendData(DataToSend);
	if (!IsSent) return false;
	const uint32 BufferSize = 1 << 12;
	const FString Response = UdpSocket->ReceiveData(ResponseTimeout, BufferSize);
	return Response == (Title + TEXT("ok"));
}

bool UQLearningTrainigComponent::ReceiveAction(const TArray<float>& State, int32 Step, int32 Episode, float& ActionRef)
{
	if (!SocketValid()) return false;
	const FString Title = TEXT("action: ");
	const FString InputsStr = StringifyArray(State);
	const FString DataToSend = FString::Printf(TEXT("%s%s %d %d"), *Title, *InputsStr, Step, Episode);
	bool IsSent = UdpSocket->SendData(DataToSend);
	if (!IsSent) return false;
	const uint32 BufferSize = 1 << 12;
	const FString Response = UdpSocket->ReceiveData(ResponseTimeout, BufferSize);
	if (Response.IsEmpty()) return false;
	if (!Response.StartsWith(Title)) return false;
	const FString ActionRaw = Response.Mid(Title.Len());
	if (ActionRaw.IsEmpty()) return false;
	uint8 ActionValue = FCString::Atoi(*ActionRaw);
	ActionRef = ActionValue == 0 ? -1 : 1;
	return true;
}

bool UQLearningTrainigComponent::SendSample(const TArray<float>& State, const TArray<float>& NextState, int32 ActionValue, bool bDone, int32 Step, int32 Episode)
{
	if (!SocketValid()) return false;
	const FString Title = TEXT("sample: ");
	const FString InputsStr = StringifyArray(State);
	const FString NextInputsStr = StringifyArray(NextState);
	const FString DataToSend = FString::Printf(TEXT("%s%s %s %d %d %d %d"), *Title, *InputsStr, *NextInputsStr, ActionValue, bDone, Step, Episode);
	bool IsSent = UdpSocket->SendData(DataToSend);
	if (!IsSent) return false;
	const uint32 BufferSize = 1 << 12;
	const FString Response = UdpSocket->ReceiveData(ResponseTimeout, BufferSize);
	return Response == (Title + TEXT("ok"));
}

bool UQLearningTrainigComponent::SaveProgress()
{
	if (!SocketValid()) return false;
	const FString Command = TEXT("save_model");
	bool IsSent = UdpSocket->SendData(Command);
	if (!IsSent) return false;
	const uint32 BufferSize = 1 << 12;
	const FString Response = UdpSocket->ReceiveData(ResponseTimeout, BufferSize);
	return Response == (Command + TEXT(": ok"));
}

FString UQLearningTrainigComponent::StringifyArray(const TArray<float>& Data) const
{
	return FString::JoinBy(Data, TEXT(" "), [] (const float& Input)
	{
		return FString::Printf(TEXT("%.5f"), Input);
	});
}

void UQLearningTrainigComponent::ShowError(float Duration, const FString& Message) const
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Red, Message);
}

void UQLearningTrainigComponent::TriggerMovement()
{
	FMovementInput PrevState = InputsState;
	FMovementInput CurrentState = Mechanism->GetUpdatedInputs();
	InputsState = CurrentState;
	int32 ActionValue = Action == -1 ? 0 : 1;
	
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, ActionValue, CurrentState, PrevState]
	{
		if (CheckValid()) return;
		bool IsSampleSent = SendSample(PrevState.ToArray(), CurrentState.ToArray(), ActionValue, IsDone, EpisodeStep, CurrentEpisode);
		if (!IsSampleSent)
			return ShowError(.3f, TEXT("Failed to send sample"));
		if (!ReceiveAction(CurrentState.ToArray(), CurrentStep, EpisodeStep, Action))
			return ShowError(5, TEXT("Action not received"));
		++EpisodeStep;
		++CurrentStep;
		
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			if (CheckValid()) return;
			if (!IsDone) return Mechanism->MoveCart(Action);
			ClearTimer();
			EpisodeStep = 0;
			++CurrentEpisode;
			Mechanism->ResetMechanism();
			bool IsAllEpisodesPassed = CurrentEpisode >= TrainingEpisodes;

			if (IsAllEpisodesPassed) FinishTraining();
			else ApplyAction();
		});
	});
}

void UQLearningTrainigComponent::SetTimer()
{
	bool bLoop = true;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UQLearningTrainigComponent::TriggerMovement, MovementFrequency, bLoop, MovementFrequency);
}

void UQLearningTrainigComponent::ClearTimer()
{
	if (!TimerHandle.IsValid()) return;
	bool bIsTimerActive = GetWorld()->GetTimerManager().IsTimerActive(TimerHandle);
	if (bIsTimerActive) GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

bool UQLearningTrainigComponent::CheckDone() const
{
	return Mechanism.IsValid() && Mechanism->ShouldReset() || EpisodeStep >= MaxActionsPerEpisode;
}

bool UQLearningTrainigComponent::CheckValid() const
{
	return !(Mechanism.IsValid() && SocketValid());
}

void UQLearningTrainigComponent::ApplyAction()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		if (CheckValid()) return;
		bool IsReceived = ReceiveAction(InputsState.ToArray(), CurrentStep, EpisodeStep, Action);
		if (!IsReceived) return ShowError(5, TEXT("Action not received"));
		
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			if (CheckValid()) return;
			InputsState = Mechanism->GetUpdatedInputs();
			Mechanism->MoveCart(Action);
			SetTimer();
			IsDone = false;
		});
	});
}

void UQLearningTrainigComponent::FinishTraining()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		if (!SaveProgress()) return ShowError(5, TEXT("Failed to save progress"));
		UdpSocket->Disconnect();
	});
}
