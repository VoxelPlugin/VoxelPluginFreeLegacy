// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelTools/Impl/VoxelToolsBaseImpl.h"
#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelTools/VoxelHardnessHandler.h"
#include "VoxelData/VoxelDataIncludes.h"

struct VOXEL_API FScopeToolsTimeLogger
{
	const char* Name;
	const int64 NumVoxels;
	const double StartTime = FPlatformTime::Seconds();

	explicit FScopeToolsTimeLogger(const char* Name, int64 NumVoxels)
		: Name(Name)
		, NumVoxels(NumVoxels)
	{
	}
	~FScopeToolsTimeLogger();
};

#define VOXEL_TOOL_FUNCTION_COUNTER(Num) FScopeToolsTimeLogger PREPROCESSOR_JOIN(EditCounter, __LINE__)(__FUNCTION__, Num); VOXEL_ASYNC_FUNCTION_COUNTER()
#define VOXEL_TOOL_SCOPE_COUNTER(Name, Num) FScopeToolsTimeLogger PREPROCESSOR_JOIN(EditCounter, __LINE__)(Name, Num); VOXEL_ASYNC_SCOPE_COUNTER(Name)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename TData>
bool FVoxelToolsBaseImpl::IsDataMultiThreaded(const TData& Data)
{
	return Data.bMultiThreadedEdits;
}

template<>
inline bool FVoxelToolsBaseImpl::IsDataMultiThreaded<FVoxelData>(const FVoxelData& Data)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////

template<typename TData>
FVoxelData& FVoxelToolsBaseImpl::GetActualData(TData& Data)
{
	return Data.Data;
}

template<>
inline FVoxelData& FVoxelToolsBaseImpl::GetActualData<FVoxelData>(FVoxelData& Data)
{
	return Data;
}

///////////////////////////////////////////////////////////////////////////////

template<typename TData>
auto* FVoxelToolsBaseImpl::GetModifiedValues(TData& Data)
{
	return static_cast<TArray<FModifiedValueDummy>*>(nullptr);
}

template<typename T>
auto* FVoxelToolsBaseImpl::GetModifiedValues(TVoxelDataImpl<T>& Data)
{
	return Data.bRecordModifiedValues ? &Data.ModifiedValues : nullptr;
}