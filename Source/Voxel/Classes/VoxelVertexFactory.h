// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VertexFactory.h"
#include "ResourceArray.h"
#include "ColorVertexBuffer.h"
#include "Components.h"
#include "PackedNormal.h"
#include "RenderUtils.h"
#include "Runtime/Launch/Resources/Version.h"


/** The vertex type used for dynamic meshes. */
struct FVoxelDynamicMeshVertex
{
	FVoxelDynamicMeshVertex() {}
	FVoxelDynamicMeshVertex( const FVector& InPosition );

	FVoxelDynamicMeshVertex(const FVector& InPosition,const FVector& InTangentX,const FVector& InTangentZ,const FVector2D& InTexCoord1,const FVector2D& InTexCoord2,const FVector2D& InTexCoord3, const FColor& InColor);

	void SetTangents( const FVector& InTangentX, const FVector& InTangentY, const FVector& InTangentZ );

	FVector GetTangentY();;

	FVector Position;
	FVector2D TextureCoordinate;
	FPackedNormal TangentX;
	FPackedNormal TangentZ;
	FColor Color;
};

/** Resource array to pass  */
class FVoxelProcMeshVertexResourceArray : public FResourceArrayInterface
{
public:
	FVoxelProcMeshVertexResourceArray(void* InData, uint32 InSize) : Data(InData), Size(InSize) {}

	const void* GetResourceData() const override { return Data; }
	uint32 GetResourceDataSize() const override { return Size; }
	void Discard() override {}
	bool IsStatic() const override { return false; }
	bool GetAllowCPUAccess() const override { return false; }
	void SetAllowCPUAccess(bool bInNeedsCPUAccess) override {}

private:
	void* Data;
	uint32 Size;
};


/** Vertex Buffer */
class FVoxelProcMeshVertexBuffer : public FVertexBuffer
{
public:
	TArray<FVoxelDynamicMeshVertex> Vertices;

	void InitRHI() override;

};

/** Index Buffer */
class FVoxelProcMeshIndexBuffer : public FIndexBuffer
{
public:
	TArray<int32> Indices;

	void InitRHI() override;
};


class FVoxelVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
public:
	FVoxelVertexFactoryShaderParameters() {}

	void Bind(const FShaderParameterMap& ParameterMap) override {}
	void Serialize(FArchive& Ar) override {}
	void SetMesh(FRHICommandList& RHICmdList, FShader* Shader, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, uint32 DataFlags) const override;
};

/** Vertex Factory */
class FVoxelVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FVoxelVertexFactory)
public:

	struct FDataType
	{
		/** The stream to read the vertex position from. */
		FVertexStreamComponent PositionComponent;

		/** The streams to read the tangent basis from. */
		FVertexStreamComponent TangentBasisComponents[2];

		/** The streams to read the texture coordinates from. */
		TArray<FVertexStreamComponent, TFixedAllocator<MAX_STATIC_TEXCOORDS / 2> > TextureCoordinates;

		/** The stream to read the shadow map texture coordinates from. */
		FVertexStreamComponent LightMapCoordinateComponent;

		/** The stream to read the vertex color from. */
		FVertexStreamComponent ColorComponent;
	};

#if ENGINE_MINOR_VERSION < 19
	FVoxelVertexFactory();
#else
	FVoxelVertexFactory(ERHIFeatureLevel::Type InFeatureLevel);
#endif

	void InitRHI() override;

	/** Init function that should only be called on render thread. */
	void Init_RenderThread(const FVoxelProcMeshVertexBuffer* VertexBuffer);

	/** Init function that can be called on any thread, and will do the right thing (enqueue command if called on main thread) */
	void Init(const FVoxelProcMeshVertexBuffer* VertexBuffer);

	void SetData(const FDataType& InData);
	
	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
#if ENGINE_MINOR_VERSION < 19
	static bool ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);
#else
	static bool ShouldCompilePermutation(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);
#endif

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment);

	static bool SupportsTessellationShaders();

	static FVertexFactoryShaderParameters* ConstructShaderParameters(EShaderFrequency ShaderFrequency);

	FORCEINLINE_DEBUGGABLE void SetColorOverrideStream(FRHICommandList& RHICmdList, const FVertexBuffer* ColorVertexBuffer) const;

protected:
	FDataType Data;
	int32 ColorStreamIndex;

	const FDataType& GetData() const { return Data; }
};

