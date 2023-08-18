// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UdpClient.generated.h"

class FSocket;

/**
 * 
 */
UCLASS()
class CARTANDPOLE_API UUdpClient : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	bool SocketValid() const;

	UFUNCTION(BlueprintCallable)
	bool SendData(const FString& Data);

	UFUNCTION(BlueprintCallable)
	FString ReceiveData(float TimeoutSeconds, int32 BufferSize);

	UFUNCTION(BlueprintCallable)
	bool CreateConnection(const FString& Host, int32 Port);

	UFUNCTION(BlueprintCallable)
	bool Disconnect();

protected:
	virtual void BeginDestroy() override;

private:
	TSharedPtr<FSocket> Socket;
};
