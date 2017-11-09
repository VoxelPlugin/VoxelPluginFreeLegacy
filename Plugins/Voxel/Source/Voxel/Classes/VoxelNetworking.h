#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "Engine.h"

class FVoxelTcpConnection
{
public:
	FSocket* const Socket;

	FVoxelTcpConnection(FSocket* const Socket);
	~FVoxelTcpConnection();

	bool SendData(TArray<uint8> Data);
	void ReceiveData(TArray<uint8>& OutData);
};

class FVoxelTcpClient
{
public:
	FVoxelTcpClient();
	~FVoxelTcpClient();

	void ConnectTcpClient(const FString& Ip, const int32 Port);

	void ReceiveData(TArray<uint8>& OutData);

	bool IsValid();

private:
	FVoxelTcpConnection* Connection;

};

class FVoxelTcpServer
{
public:
	FVoxelTcpServer();
	~FVoxelTcpServer();

	void StartTcpServer(const FString& Ip, const int32 Port);

	bool Accept(FSocket* NewSocket, const FIPv4Endpoint& Endpoint);

	bool SendData(TArray<uint8> Data);

	bool IsValid();

private:
	FTcpListener* TcpListener;
	TArray<FVoxelTcpConnection*> Connections;
};