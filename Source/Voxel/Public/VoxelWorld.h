// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRuntime.h"
#include "VoxelRuntimeActor.h"
#include "VoxelCoordinatesProvider.h"
#include "VoxelWorldCreateInfo.h"
#include "VoxelEditorDelegatesInterface.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelWorld.generated.h"

class UVoxelGeneratorCache;
class UVoxelLineBatchComponent;
class UVoxelWorldRootComponent;
class UVoxelPlaceableItemActorHelper;

/**
 * Voxel World actor class
 */
UCLASS()
class VOXEL_API AVoxelWorld
	: public AVoxelRuntimeActor
	, public FVoxelCoordinatesProvider
	, public IVoxelEditorDelegatesInterface
{
	GENERATED_BODY()
		
public:
	class FGameThreadTasks
	{
	public:
		void AddTask(const TFunction<void()>& Task)
		{
			Tasks.Enqueue(Task);
		}

	private:
		TQueue<TFunction<void()>, EQueueMode::Mpsc> Tasks;

		void Flush();

		friend AVoxelWorld;
	};
	
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGenerateWorld);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldLoaded);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldDestroyed);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMaxFoliageInstancesReached);

	// Called when generating the world, right after it's created
	// Bind this to add data items, or to do something right after the world is created
	UPROPERTY(BlueprintAssignable)
	FOnGenerateWorld OnGenerateWorld;

	UPROPERTY(BlueprintAssignable)
	FOnWorldLoaded OnWorldLoaded;

	// Called right before destroying the world. Use this if you want to save data
	UPROPERTY(BlueprintAssignable)
	FOnWorldDestroyed OnWorldDestroyed;

	// Called when max foliage instances is reached
	UPROPERTY(BlueprintAssignable)
	FOnMaxFoliageInstancesReached OnMaxFoliageInstancesReached;

protected:
	UPROPERTY(Category = "Voxel", VisibleAnywhere, BlueprintReadOnly)
	UVoxelWorldRootComponent* WorldRoot = nullptr;
	
	UPROPERTY()
	UVoxelLineBatchComponent* LineBatchComponent = nullptr;

public:
	UVoxelWorldRootComponent& GetWorldRoot() const { check(WorldRoot); return *WorldRoot; }
	UVoxelLineBatchComponent& GetLineBatchComponent() const { check(LineBatchComponent); return *LineBatchComponent; }

public:
	// Automatically loaded on creation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Save", meta = (Recreate))
	UVoxelWorldSaveObject* SaveObject = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Save")
	FString SaveFilePath;

	// If true, will save the world to SaveFilePath each time it's saved to the save object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Save")
	bool bAutomaticallySaveToFile = false;

	// If true, will add the current time & date to the filepath when saving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Save")
	bool bAppendDateToSavePath = false;

	//////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Bake")
	bool bRecomputeNormalsBeforeBaking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Bake")
	UStaticMesh* BakedMeshTemplate = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Bake")
	TSubclassOf<UStaticMeshComponent> BakedMeshComponentTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Bake", meta = (RelativeToGameContentDir))
	FFilePath BakedDataPath = { "/Game/VoxelStaticData" };

	//////////////////////////////////////////////////////////////////////////////
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Preview", meta = (Recreate))
	bool bEnableFoliageInEditor = true;

	// Turns this off if there's a significant lag when changing material properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Preview")
	bool bAutomaticallyRefreshMaterials = true;

	// Turns this off if there's a significant lag when changing foliage properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Preview")
	bool bAutomaticallyRefreshFoliage = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Preview", meta = (DisplayName = "New Scale"))
	FVector EditorOnly_NewScale = FVector(2, 2, 2);

	//////////////////////////////////////////////////////////////////////////////

	UPROPERTY()
	int32 NumberOfThreads_DEPRECATED;

	VOXEL_DEPRECATED(1.2, "Seeds are now regular generator parameters")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - General", meta = (DisplayName = "Seeds (DEPRECATED)"))
	TMap<FName, int32> Seeds;
	
public:
	AVoxelWorld();

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void CreateWorld(FVoxelWorldCreateInfo Info);
	void CreateWorld() { CreateWorld({}); }

	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void DestroyWorld();
	
public:
	FVoxelIntBox GetWorldBounds() const;
	const TVoxelSharedPtr<FGameThreadTasks>& GetGameThreadTasks() const { return GameThreadTasks; }
	FVoxelRuntime& GetRuntime() const { return *Runtime; }
	EVoxelPlayType GetPlayType() const { return PlayType; }

	template<typename T>
	TVoxelSharedPtr<T> GetSubsystem() const
	{
		return GetRuntime().GetSubsystem<T>();
	}
	template<typename T>
	T& GetSubsystemChecked() const
	{
		return GetRuntime().GetSubsystemChecked<T>();
	}
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetGeneratorObject(UVoxelGenerator* NewGenerator);
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetGeneratorClass(TSubclassOf<UVoxelGenerator> NewGeneratorClass);

	// Set the render octree depth
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Size")
	void SetRenderOctreeDepth(int32 NewDepth);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Size")
	void SetWorldSize(int32 NewWorldSizeInVoxels);
	void SetWorldSize(uint32 NewWorldSizeInVoxels);

public:
	// Is this world created?
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	inline bool IsCreated() const { return Runtime.IsValid(); }

	// Has the VoxelRenderer finished loading?
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	inline bool IsLoaded() const { return bIsLoaded; }

public:
	/**
	 * Convert position from world space to voxel space
	 * @param	Position	Position in world space
	 * @param	Rounding	How to round
	 * @return	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Coordinates", meta = (DisplayName = "World Position to Voxel", AdvancedDisplay = "Rounding"))
	FIntVector K2_GlobalToLocal(FVector Position, EVoxelWorldCoordinatesRounding Rounding = EVoxelWorldCoordinatesRounding::RoundToNearest) const
	{
		return GlobalToLocal(Position, Rounding);
	}

	UFUNCTION(BlueprintCallable, Category = "Voxel|Coordinates", meta = (DisplayName = "World Position to Voxel Float"))
	FVector K2_GlobalToLocalFloat(const FVector& Position) const
	{
		return GlobalToLocalFloat(Position).ToFloat();
	}
	
	/**
	 * Convert position from voxel space to world space
	 * @param	Position	Position in voxel space
	 * @return	Position in world space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Coordinates", meta = (DisplayName = "Voxel Position to World"))
	FVector K2_LocalToGlobal(const FIntVector& Position) const
	{
		return LocalToGlobal(Position).ToFloat();
	}
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Coordinates", meta = (DisplayName = "Voxel Position to World Float"))
	FVector K2_LocalToGlobalFloat(const FVector& Position) const
	{
		return LocalToGlobalFloat(Position).ToFloat();
	}
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Coordinates", meta = (DisplayName = "Voxel Bounds to World"))
	FBox K2_LocalToGlobalBounds(const FVoxelIntBox& Bounds) const
	{
		return LocalToGlobalBounds(Bounds);
	}
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Coordinates", meta = (DisplayName = "World Bounds to Voxel"))
	FVoxelIntBox K2_GlobalToLocalBounds(const FBox& Bounds) const
	{
		return GlobalToLocalBounds(Bounds);
	}
	
	/**
	 * Get the 8 neighbors in voxel space of GlobalPosition
	 * @param	GlobalPosition	The position in world space
	 * @return	The 8 neighbors in voxel space 
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Coordinates")
	TArray<FIntVector> GetNeighboringPositions(const FVector& GlobalPosition) const;

	// The generator cache allows to reuse generator objects
	// This is required for DataItemActors to allow for smaller update when moving them
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	UVoxelGeneratorCache* GetGeneratorCache() const;

	// Used to init generators
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	FVoxelGeneratorInit GetGeneratorInit() const;
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Multiplayer")
	UVoxelMultiplayerInterface* CreateMultiplayerInterfaceInstance();
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Multiplayer")
	UVoxelMultiplayerInterface* GetMultiplayerInterfaceInstance() const;

	// Can be called at runtime
	UFUNCTION(BlueprintCallable, Category="Voxel|Collision")
	void SetCollisionResponseToChannel(ECollisionChannel Channel, ECollisionResponse NewResponse);

public:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void UnregisterAllComponents(bool bForReregister = false) override;
#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif // WITH_EDITOR
	//~ End AActor Interface

	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyThatWillChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif // WITH_EDITOR
	//~ End UObject Interface

	//~ Begin FVoxelCoordinatesProvider Interface
	// You can override this in a child class if you want to return a more precise transform than the actor transform
	// eg, a planet rotation
	virtual FVoxelTransform GetVoxelTransform() const override { return GetTransform(); }
	virtual v_flt GetVoxelSize() const override { return VoxelSize; }
	//~ End FVoxelCoordinatesProvider Interface	

	void UpdateCollisionProfile();

	
private:
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	AActor* VoxelWorldEditor;
#endif

	UPROPERTY(Transient)
	mutable UVoxelMultiplayerInterface* MultiplayerInterfaceInstance;

	UPROPERTY(Transient)
	mutable UVoxelGeneratorCache* GeneratorCache = nullptr;

	UPROPERTY(Transient)
	UVoxelPlaceableItemActorHelper* PlaceableItemActorHelper = nullptr;
	
	UPROPERTY()
	bool bIsToggled = false;
	
	bool bIsLoaded = false;
	bool bSimulatePhysicsOnceLoaded = false;
	EVoxelPlayType PlayType = EVoxelPlayType::Game;
	double TimeOfCreation = 0;
	FVoxelTransform LastTransform;

	bool bRegenerateFoliageOnNextFrame = false;

#if WITH_EDITOR
	// Temporary variable set in PreEditChange to avoid re-registering proc meshes
	bool bDisableComponentUnregister = false;
#endif

private:
	// TODO Subsystem
	TVoxelSharedPtr<FGameThreadTasks> GameThreadTasks;
	TVoxelSharedPtr<FVoxelRuntime> Runtime;
	
private:
	void OnWorldLoadedCallback();

	void CreateWorldInternal(const FVoxelWorldCreateInfo& Info);
	void DestroyWorldInternal();
	void DestroyVoxelComponents();

public:
	void LoadFromSaveObject();
	void ApplyPlaceableItems();

	void ApplyCollisionSettingsToRoot() const;

	void RecreateRender();
	void RecreateFoliage();
	void RecreateAll(const FVoxelWorldCreateInfo& Info);

	FVoxelRuntimeSettings GetRuntimeSettings() const;

#if WITH_EDITOR
	FSimpleMulticastDelegate OnPropertyChanged;
	FSimpleMulticastDelegate OnPropertyChanged_Interactive;

	void Toggle();
	void CreateInEditor(const FVoxelWorldCreateInfo& Info = {});
	void SaveData();
	void LoadFromSaveObjectEditor();
	bool SaveToFile(const FString& Path, FText& Error);
	bool LoadFromFile(const FString& Path, FText& Error);
	FString GetDefaultFilePath() const;
	
	//~ Begin IVoxelEditorDelegatesInterface Interface
	virtual void OnPreSaveWorld(uint32 SaveFlags, UWorld* World) override;
	virtual void OnPreBeginPIE(bool bIsSimulating) override;
	virtual void OnEndPIE(bool bIsSimulating) override;
	virtual void OnPrepareToCleanseEditorObject(UObject* Object) override;
	virtual void OnPreExit() override;
	virtual void OnApplyObjectToActor(UObject* Object, AActor* Actor) override;
	//~ End IVoxelEditorDelegatesInterface Interface
#endif
};

#if WITH_EDITOR
class VOXEL_API IVoxelWorldEditor
{
public:
	virtual ~IVoxelWorldEditor() = default;

	virtual UVoxelWorldSaveObject* CreateSaveObject() = 0;
	virtual UClass* GetVoxelWorldEditorClass() = 0;
	virtual void RegisterTransaction(AVoxelWorld* VoxelWorld, FName Name) = 0;

public:
	// Sets the voxel world editor implementation.*
	static void SetVoxelWorldEditor(TSharedPtr<IVoxelWorldEditor> InVoxelWorldEditor);
	inline static IVoxelWorldEditor* GetVoxelWorldEditor() { return VoxelWorldEditor.Get(); }

private:
	// Ptr to interface to voxel editor operations.
	static TSharedPtr<IVoxelWorldEditor> VoxelWorldEditor;
};
#endif