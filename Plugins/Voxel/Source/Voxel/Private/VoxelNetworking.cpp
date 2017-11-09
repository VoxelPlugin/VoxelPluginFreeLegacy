#include "VoxelPrivatePCH.h"
#include "VoxelNetworking.h"

FVoxelTcpSender::FVoxelTcpSender()
	: Socket(nullptr)
{
}

FVoxelTcpSender::~FVoxelTcpSender()
{
	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}
}

bool FVoxelTcpSender::StartTCPSender(const FString& Ip, const int32 Port)
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

bool FVoxelTcpSender::SendData(TArray<uint8> Data)
{
	if (!Socket)
	{
		UE_LOG(LogTemp, Error, TEXT("No sender socket"));
		return false;
	}

	int32 BytesSent = 0;
	FArrayWriter Writer;

	Writer << Data;

	bool bSuccess = Socket->Send(Writer.GetData(), Writer.Num(), BytesSent);
	
	UE_LOG(LogTemp, Log, TEXT("Bytes sent: %d. Success: %d"), BytesSent, bSuccess);


	return true;
}





FVoxelTcpListener::FVoxelTcpListener()
	: Socket(nullptr)
{

}

FVoxelTcpListener::~FVoxelTcpListener()
{
	delete TcpListener;
}

void FVoxelTcpListener::StartTCPListener(const FString& Ip, const int32 Port)
{
	FIPv4Address Addr;
	FIPv4Address::Parse(Ip, Addr);

	FIPv4Endpoint Endpoint(Addr, Port);

	TcpListener = new FTcpListener(Endpoint);

	TcpListener->OnConnectionAccepted().BindRaw(this, &FVoxelTcpListener::Accept);
}

bool FVoxelTcpListener::ReceiveData(TArray<uint8>& OutData)
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

			ReceivedData << OutData;

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%d bytes received"), BytesRead));
		}

		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No listener socket"));
		return false;
	}
}

bool FVoxelTcpListener::Accept(FSocket* NewSocket, const FIPv4Endpoint& Endpoint)
{
	Socket = NewSocket;
	return true;
}