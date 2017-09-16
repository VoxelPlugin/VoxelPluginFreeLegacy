#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "Engine.h"
#include "VoxelNetworking.generated.h"

class VoxelNetworking
{

};










USTRUCT(BlueprintType)
struct FAnyCustomData
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joy Color")
		FString Name = "Victory!";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joy Color")
		int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joy Color")
		float Scale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joy Color")
		FLinearColor Color = FLinearColor::Red;

	FAnyCustomData()
	{
	}
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FAnyCustomData& TheStruct)
{
	Ar << TheStruct.Name;
	Ar << TheStruct.Count;
	Ar << TheStruct.Scale;
	Ar << TheStruct.Color;

	return Ar;
}












UCLASS()
class AVoxelTcpSender : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	TSharedPtr<FInternetAddr>	RemoteAddr;
	FSocket* SenderSocket;

	UFUNCTION(BlueprintCallable)
		bool StartUDPSender(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort);

	UFUNCTION(BlueprintCallable)
		bool RamaUDPSender_SendString(FString ToSend);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rama UDP Sender")
		bool ShowOnScreenDebugMessages;


	//ScreenMsg
	FORCEINLINE void ScreenMsg(const FString& Msg)
	{
		if (!ShowOnScreenDebugMessages) return;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *Msg);
	}
	FORCEINLINE void ScreenMsg(const FString& Msg, const float Value)
	{
		if (!ShowOnScreenDebugMessages) return;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %f"), *Msg, Value));
	}
	FORCEINLINE void ScreenMsg(const FString& Msg, const FString& Msg2)
	{
		if (!ShowOnScreenDebugMessages) return;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %s"), *Msg, *Msg2));
	}


public:

	/** Called whenever this actor is being removed from a level */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
















UCLASS()
class AVoxelTcpListener : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	FTcpListener* TcpListener;

	void Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	UFUNCTION(BlueprintCallable)
		void TCPSocketListener();

	UFUNCTION(BlueprintCallable)
		bool StartUDPReceiver(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort);

	//ScreenMsg
	FORCEINLINE void ScreenMsg(const FString& Msg)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *Msg);
	}
	FORCEINLINE void ScreenMsg(const FString& Msg, const float Value)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %f"), *Msg, Value));
	}
	FORCEINLINE void ScreenMsg(const FString& Msg, const FString& Msg2)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %s"), *Msg, *Msg2));
	}


public:

	/** Called whenever this actor is being removed from a level */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};