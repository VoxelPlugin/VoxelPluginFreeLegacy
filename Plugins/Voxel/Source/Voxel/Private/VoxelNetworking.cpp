#include "VoxelPrivatePCH.h"
#include "VoxelNetworking.h"


FVoxelTcpConnection::FVoxelTcpConnection(FSocket* const Socket)
	: Socket(Socket)
{
	check(Socket);
}

FVoxelTcpConnection::~FVoxelTcpConnection()
{
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
}

bool FVoxelTcpConnection::SendData(TArray<uint8> Data)
{
	int32 BytesSent = 0;
	FArrayWriter Writer;

	Writer << Data;

	bool bSuccess = Socket->Send(Writer.GetData(), Writer.Num(), BytesSent);

	//UE_LOG(LogTemp, Log, TEXT("Bytes sent: %d. Success: %d"), BytesSent, bSuccess);


	return bSuccess;
}

void FVoxelTcpConnection::ReceiveData(TArray<uint8>& OutData)
{
	uint32 PendingDataSize = 0;
	while (Socket->HasPendingData(PendingDataSize))
	{
		FArrayReader ReceivedData = FArrayReader(true);
		ReceivedData.Init(0, FMath::Min(PendingDataSize, 65507u));

		int32 BytesRead = 0;
		Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);

		ReceivedData << OutData;

		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%d bytes received"), BytesRead));
	}
}





FVoxelTcpClient::FVoxelTcpClient()
	: Connection(nullptr)
{

}

FVoxelTcpClient::~FVoxelTcpClient()
{
	if (Connection)
	{
		delete Connection;
	}
}

void FVoxelTcpClient::ConnectTcpClient(const FString& Ip, const int32 Port)
{
	//Create Remote Address.
	TSharedPtr<FInternetAddr> RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool bIsValid;
	RemoteAddr->SetIp(*Ip, bIsValid);
	RemoteAddr->SetPort(Port);

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("IP address was not valid"));
		return;
	}

	FIPv4Endpoint Endpoint(RemoteAddr);

	FSocket* Socket = FTcpSocketBuilder(TEXT("RemoteConnection"));
	if (Socket)
	{
		if (!Socket->Connect(*Endpoint.ToInternetAddr()))
		{
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
			Socket = nullptr;
			return;
		}
		else
		{
			if (Connection)
			{
				delete Connection;
			}
			Connection = new FVoxelTcpConnection(Socket);
		}
	}
}

void FVoxelTcpClient::ReceiveData(TArray<uint8>& OutData)
{
	if (Connection)
	{
		Connection->ReceiveData(OutData);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Client not connected"));
	}
}

bool FVoxelTcpClient::IsValid()
{
	return Connection;
}





FVoxelTcpServer::FVoxelTcpServer()
	: TcpListener(nullptr)
{

}

FVoxelTcpServer::~FVoxelTcpServer()
{
	for (auto Connection : Connections)
	{
		delete Connection;
	}
	delete TcpListener;
}

void FVoxelTcpServer::StartTcpServer(const FString& Ip, const int32 Port)
{
	if (TcpListener)
	{
		delete TcpListener;
	}

	FIPv4Address Addr;
	FIPv4Address::Parse(Ip, Addr);

	FIPv4Endpoint Endpoint(Addr, Port);

	TcpListener = new FTcpListener(Endpoint);

	TcpListener->OnConnectionAccepted().BindRaw(this, &FVoxelTcpServer::Accept);
}

bool FVoxelTcpServer::Accept(FSocket* NewSocket, const FIPv4Endpoint& Endpoint)
{
	Connections.Add(new FVoxelTcpConnection(NewSocket));

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Connected!"));
	return true;
}

bool FVoxelTcpServer::SendData(TArray<uint8> Data)
{
	bool bSuccess = true;
	for (auto Socket : Connections)
	{
		bSuccess = bSuccess && Socket->SendData(Data);
	}
	return bSuccess;
}

bool FVoxelTcpServer::IsValid()
{
	return Connections.Num();
}
