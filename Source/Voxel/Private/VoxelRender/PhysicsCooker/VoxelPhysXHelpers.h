// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
#include "Px.h"
#include "PxVec4.h"
#include "PxTriangleMesh.h"
THIRD_PARTY_INCLUDES_END

// The code below is using PhysX class layouts. PhysX is licensed under the following terms:

//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

namespace FVoxelPhysXHelpers
{
	using namespace physx;

	namespace Cm
	{
		class RefCountable
		{
		public:
			virtual ~RefCountable() {}

		private:
			volatile PxI32 mRefCount;
		};

	}

	namespace Ps
	{
		class UserAllocated
		{
		};
	}
	
	class CenterExtents : public Ps::UserAllocated
	{
	public:
		PxVec3 mCenter;
		PxVec3 mExtents;
	};

	class TriangleMesh : public PxTriangleMesh, public Ps::UserAllocated, public Cm::RefCountable
	{
	public:
		virtual ~TriangleMesh() {}

		PxU32 mNbVertices;
		PxU32 mNbTriangles;
		PxVec3* mVertices;
		void* mTriangles; //!< 16 (<= 0xffff #vertices) or 32 bit trig indices (mNbTriangles * 3)
		// 16 bytes block

		// PT: WARNING: bounds must be followed by at least 32bits of data for safe SIMD loading
		CenterExtents mAABB;
		PxU8* mExtraTrigData; // one per trig
		PxReal mGeomEpsilon; //!< see comments in cooking code referencing this variable
		// 16 bytes block
		/*
		low 3 bits (mask: 7) are the edge flags:
		b001 = 1 = ignore edge 0 = edge v0-->v1
		b010 = 2 = ignore edge 1 = edge v0-->v2
		b100 = 4 = ignore edge 2 = edge v1-->v2
		*/
		PxU8 mFlags;					//!< Flag whether indices are 16 or 32 bits wide
										//!< Flag whether triangle adajacencies are build
		PxU16* mMaterialIndices;		//!< the size of the array is numTriangles.
		PxU32* mFaceRemap;				//!< new faces to old faces mapping (after cleaning, etc). Usage: old = faceRemap[new]
		PxU32* mAdjacencies;			//!< Adjacency information for each face - 3 adjacent faces

		class GuMeshFactory* mMeshFactory; // PT: changed to pointer for serialization

		// GRB data -------------------------
		void* mGRB_triIndices;				//!< GRB: GPU-friendly tri indices [uint4]

		void* mGRB_triAdjacencies;			//!< GRB: adjacency data, with BOUNDARY and NONCONVEX flags (flags replace adj indices where applicable)
		PxU32* mGRB_vertValency;			//!< GRB: number of adjacent vertices to a vertex
		PxU32* mGRB_adjVertStart;			//!< GRB: offset for each vertex in the adjacency list
		PxU32* mGRB_adjVertices;			//!< GRB: list of adjacent vertices

		PxU32 mGRB_meshAdjVerticiesTotal;	//!< GRB: total number of indices in the 'mGRB_adjVertices'
		PxU32* mGRB_faceRemap;				//!< GRB : gpu to cpu triangle indice remap
		void* mGRB_BV32Tree;				//!< GRB: BV32 tree
		// End of GRB data ------------------

	};

	PX_ALIGN_PREFIX(16)
	struct RTreePage
	{
		typedef PxF32 RTreeValue;

#define RTREE_N 4
		RTreeValue minx[RTREE_N];
		RTreeValue miny[RTREE_N];
		RTreeValue minz[RTREE_N];
		RTreeValue maxx[RTREE_N];
		RTreeValue maxy[RTREE_N];
		RTreeValue maxz[RTREE_N];
		PxU32 ptrs[RTREE_N];
#undef RTREE_N
	} PX_ALIGN_SUFFIX(16);

	PX_ALIGN_PREFIX(16)
	struct RTree
	{
		PxVec4 mBoundsMin, mBoundsMax, mInvDiagonal, mDiagonalScaler; // 16
		PxU32 mPageSize;
		PxU32 mNumRootPages;
		PxU32 mNumLevels;
		PxU32 mTotalNodes; // 16
		PxU32 mTotalPages;
		PxU32 mFlags;
		RTreePage* mPages;
	} PX_ALIGN_SUFFIX(16);

	class RTreeTriangleMesh : public TriangleMesh
	{
	public:
		virtual ~RTreeTriangleMesh() {}

		RTree mRTree;
	};

	inline uint32 GetAllocatedSize(PxTriangleMesh& InTriangleMesh)
	{
		auto& TriangleMesh = static_cast<RTreeTriangleMesh&>(InTriangleMesh);

		uint64 Size = 0;
		Size += sizeof(RTreeTriangleMesh);

		if (TriangleMesh.mVertices)
		{
			Size += TriangleMesh.mNbVertices * sizeof(PxVec3);
		}
		if (TriangleMesh.mTriangles)
		{
			const PxU32 triangleSize = TriangleMesh.mFlags & PxTriangleMeshFlag::e16_BIT_INDICES ? sizeof(PxU16) : sizeof(PxU32);
			Size += TriangleMesh.mNbTriangles * 3 * triangleSize;
		}
		if (TriangleMesh.mExtraTrigData)
		{
			Size += TriangleMesh.mNbTriangles * sizeof(PxU8);
		}
		if (TriangleMesh.mMaterialIndices)
		{
			Size += TriangleMesh.mNbTriangles * sizeof(PxU16);
		}
		if (TriangleMesh.mFaceRemap)
		{
			Size += TriangleMesh.mNbTriangles * sizeof(PxU32);
		}
		if (TriangleMesh.mAdjacencies)
		{
			Size += TriangleMesh.mNbTriangles * sizeof(PxU32) * 3;
		}
		ensure(Size < MAX_int32);
		return Size;
	}
}