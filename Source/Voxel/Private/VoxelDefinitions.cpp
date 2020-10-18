// Copyright 2020 Phyronnaz

#include "VoxelDefinitions.h"
#include "VoxelLog.h"
#include "VoxelStats.h"
#include "VoxelFeedbackContext.h"
#include "VoxelIntBox.h"
#include "VoxelItemStack.h"
#include "VoxelEditorDelegates.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

#include "Logging/LogMacros.h"
#include "Serialization/CustomVersion.h"

static_assert(FVoxelUtilities::IsPowerOfTwo(RENDER_CHUNK_SIZE), "RENDER_CHUNK_SIZE must be a power of 2");
static_assert(FVoxelUtilities::IsPowerOfTwo(DATA_CHUNK_SIZE), "DATA_CHUNK_SIZE must be a power of 2");

#if VOXEL_MATERIAL_ENABLE_UV1 && !VOXEL_MATERIAL_ENABLE_UV0
#error "Error"
#endif
#if VOXEL_MATERIAL_ENABLE_UV2 && !VOXEL_MATERIAL_ENABLE_UV1
#error "Error"
#endif
#if VOXEL_MATERIAL_ENABLE_UV3 && !VOXEL_MATERIAL_ENABLE_UV2
#error "Error"
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DEFINE_LOG_CATEGORY(LogVoxel);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if ENABLE_VOXEL_MEMORY_STATS
TMap<const TCHAR*, FVoxelMemoryCounterRef>& GetVoxelMemoryCounters()
{
	static TMap<const TCHAR*, FVoxelMemoryCounterRef> GVoxelMemoryCounters;
	return GVoxelMemoryCounters;
}

FVoxelMemoryCounterStaticRef::FVoxelMemoryCounterStaticRef(const TCHAR* Name, const FVoxelMemoryCounterRef& Ref)
{
	GetVoxelMemoryCounters().Add(Name, Ref);
}
#endif

DEFINE_VOXEL_MEMORY_STAT(STAT_TotalVoxelMemory);

static FFeedbackContext* GVoxelFeedbackContext = nullptr;

void SetVoxelFeedbackContext(class FFeedbackContext& FeedbackContext)
{
	GVoxelFeedbackContext = &FeedbackContext;
}

FVoxelScopedSlowTask::FVoxelScopedSlowTask(float InAmountOfWork, const FText& InDefaultMessage, bool bInEnabled)
	: FScopedSlowTask(InAmountOfWork, InDefaultMessage, bInEnabled, GVoxelFeedbackContext ? *GVoxelFeedbackContext : *GWarn)
{

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// +/- 1024: prevents integers overflow
FVoxelIntBox const FVoxelIntBox::Infinite = FVoxelIntBox(FIntVector(MIN_int32 + 1024), FIntVector(MAX_int32 - 1024));

const FVoxelPlaceableItemHolder EmptyVoxelPlaceableItemHolder;
FVoxelItemStack FVoxelItemStack::Empty = FVoxelItemStack(EmptyVoxelPlaceableItemHolder);

const FVoxelVector FVoxelVector::ZeroVector(0.0f, 0.0f, 0.0f);
const FVoxelVector FVoxelVector::OneVector(1.0f, 1.0f, 1.0f);
const FVoxelVector FVoxelVector::UpVector(0.0f, 0.0f, 1.0f);
const FVoxelVector FVoxelVector::DownVector(0.0f, 0.0f, -1.0f);
const FVoxelVector FVoxelVector::ForwardVector(1.0f, 0.0f, 0.0f);
const FVoxelVector FVoxelVector::BackwardVector(-1.0f, 0.0f, 0.0f);
const FVoxelVector FVoxelVector::RightVector(0.0f, 1.0f, 0.0f);
const FVoxelVector FVoxelVector::LeftVector(0.0f, -1.0f, 0.0f);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define VOXEL_DEBUG_DELEGATE(Type) \
	template<> \
	VOXEL_API FVoxelDebug::TDelegate<Type>& FVoxelDebug::GetDelegate<Type>() \
	{ \
		static TDelegate<Type> Delegate; \
		return Delegate; \
	}

VOXEL_DEBUG_DELEGATE(FVoxelValue);
VOXEL_DEBUG_DELEGATE(float);

#undef VOXEL_DEBUG_DELEGATE

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint32& FVoxelRangeFailStatus::GetTlsSlot()
{
	// Not inline, else it's messed up across modules
	static uint32 TlsSlot = 0xFFFFFFFF;
	return TlsSlot;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VOXEL_API FString VoxelStats_RemoveLambdaFromFunctionName(const FString& FunctionName)
{
#if PLATFORM_WINDOWS
	ensure(FunctionName.EndsWith("::operator ()"));

	TArray<FString> Array;
	FunctionName.ParseIntoArray(Array, TEXT("::"));

	// operator()
	if (ensure(Array.Num() > 1)) Array.Pop(false);
	// <lambda XXXXXXX>
	if (ensure(Array.Num() > 1)) Array.Pop(false);

	return FString::Join(Array, TEXT("::"));
#else
	return FunctionName;
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelEditorDelegates::FFixVoxelLandscapeMaterial FVoxelEditorDelegates::FixVoxelLandscapeMaterial;