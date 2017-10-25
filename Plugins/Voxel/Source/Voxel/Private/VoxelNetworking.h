#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "Engine.h"
#include "VoxelNetworking.generated.h"

UCLASS()
class AVoxelTcpSender : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	TSharedPtr<FInternetAddr> RemoteAddr;
	FSocket* Socket;

	UFUNCTION(BlueprintCallable)
		bool StartTCPSender(const FString& Ip, const int32 Port);

	UFUNCTION(BlueprintCallable)
		bool SendString(FString ToSend);

public:

	/** Called whenever this actor is being removed from a level */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

class AcceptClass
{
	FSocket*& SocketRef;

public:
	AcceptClass(FSocket*& SocketRef) : SocketRef(SocketRef) {}

	bool Accept(FSocket* Socket, const FIPv4Endpoint& Endpoint)
	{
		SocketRef = Socket;
		return true;
	}
};

UCLASS()
class AVoxelTcpListener : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	FTcpListener* TcpListener;
	FSocket* Socket;
	AcceptClass* AcceptClassInstance;

	UFUNCTION(BlueprintCallable)
		void StartTCPListener(const FString& Ip, const int32 Port);

	UFUNCTION(BlueprintCallable)
		bool ReceiveMessages();

public:

	/** Called whenever this actor is being removed from a level */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};