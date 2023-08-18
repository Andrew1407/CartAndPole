// Fill out your copyright notice in the Description page of Project Settings.


#include "Connection/UdpClient.h"
#include "Networking.h"

void UUdpClient::BeginDestroy()
{
    Disconnect();
    Super::BeginDestroy();
}

bool UUdpClient::SocketValid() const
{
    return Socket.IsValid() && Socket->GetConnectionState() == SCS_Connected;
}

bool UUdpClient::SendData(const FString& Data)
{
    if (!SocketValid()) return false;
    TArray<uint8> MessageArray;
    FTCHARToUTF8 ConvertedMsg(*Data);
    MessageArray.Append((uint8*)ConvertedMsg.Get(), ConvertedMsg.Length());
    int32 BytesSent = 0;
    return Socket->Send(MessageArray.GetData(), MessageArray.Num(), BytesSent);
}

FString UUdpClient::ReceiveData(float TimeoutSeconds, int32 BufferSize)
{
    if (!SocketValid()) return {};
    TUniquePtr<uint8[]> Buffer = MakeUnique<uint8[]>(BufferSize);
    int32 BytesReceived = 0;
    int32 BytesRead = 0;
    bool IsRed = Socket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(TimeoutSeconds));
    if (!IsRed) return {};                              // Handle timeout or error condition
    BytesRead = Socket->Recv(Buffer.Get(), BufferSize, BytesReceived);
    if (BytesRead <= 0) return {};                      // No response received from the server
    return UTF8_TO_TCHAR(reinterpret_cast<const ANSICHAR*>(Buffer.Get()));
}

bool UUdpClient::CreateConnection(const FString& Host, int32 Port)
{
    if (SocketValid()) return false;
    Socket = MakeShareable(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, TEXT("UDPClientSocket")));
    TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    bool IsIpValid;
    RemoteAddress->SetIp(*Host, IsIpValid);
    RemoteAddress->SetPort(Port);
    if (IsIpValid) IsIpValid = Socket->Connect(*RemoteAddress);
    return IsIpValid;
}

bool UUdpClient::Disconnect()
{
    if (!SocketValid()) return false;
    Socket->Close();
    Socket.Reset();
    return true;
}
