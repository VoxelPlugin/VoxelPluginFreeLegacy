#include "VoxelPrivatePCH.h"
#include "VoxelNetworking.h"

AVoxelTcpSender::AVoxelTcpSender(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Socket = nullptr;
}

void AVoxelTcpSender::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}
}

bool AVoxelTcpSender::StartTCPSender(const FString& Ip, const int32 Port)
{
	//Create Remote Address.
	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool bIsValid;
	RemoteAddr->SetIp(*Ip, bIsValid);
	RemoteAddr->SetPort(Port);

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("IP address was not valid"));
		return false;
	}

	FIPv4Endpoint Endpoint(RemoteAddr);

	if (!Socket)
	{
		Socket = FTcpSocketBuilder(TEXT("RemoteConnection"));
		if (Socket)
		{
			if (!Socket->Connect(*Endpoint.ToInternetAddr()))
			{
				ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
				Socket = nullptr;
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool AVoxelTcpSender::SendString(FString ToSend)
{
	if (!Socket)
	{
		UE_LOG(LogTemp, Error, TEXT("No sender socket"));
		return false;
	}

	int32 BytesSent = 0;
	FArrayWriter Writer;

	Writer << ToSend;

	bool bSuccess = Socket->Send(Writer.GetData(), Writer.Num(), BytesSent);

	UE_LOG(LogTemp, Error, TEXT("Bytes sent: %d. Success: %d"), BytesSent, bSuccess);


	return true;
}






AVoxelTcpListener::AVoxelTcpListener(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AcceptClassInstance = new AcceptClass(Socket);
	Socket = nullptr;
}

void AVoxelTcpListener::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	delete TcpListener;
	TcpListener = nullptr;
}

void AVoxelTcpListener::StartTCPListener(const FString& Ip, const int32 Port)
{
	FIPv4Address Addr;
	FIPv4Address::Parse(Ip, Addr);

	FIPv4Endpoint Endpoint(Addr, Port);

	TcpListener = new FTcpListener(Endpoint);

	TcpListener->OnConnectionAccepted().BindRaw(AcceptClassInstance, &AcceptClass::Accept);
}

bool AVoxelTcpListener::ReceiveMessages()
{
	if (Socket)
	{
		uint32 PendingDataSize = 0;
		while (Socket->HasPendingData(PendingDataSize))
		{
			FArrayReader ReceivedData = FArrayReader(true);
			ReceivedData.Init(0, FMath::Min(PendingDataSize, 65507u));

			int32 BytesRead = 0;
			Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);

			FString Message;
			ReceivedData << Message;

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
		}

		return true;
	}
	else
	{
		return false;
	}
}
