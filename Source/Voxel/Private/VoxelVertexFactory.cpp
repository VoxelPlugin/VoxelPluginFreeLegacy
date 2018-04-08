// Copyright 2018 Phyronnaz

#include "VoxelVertexFactory.h"
#include "MeshBatch.h"


FVoxelDynamicMeshVertex::FVoxelDynamicMeshVertex(const FVector& InPosition) :
	Position(InPosition),
	TextureCoordinate(FVector2D::ZeroVector),
	TangentX(FVector(1, 0, 0)),
	TangentZ(FVector(0, 0, 1)),
	Color(FColor(255, 255, 255))
{
	// basis determinant default to +1.0
	TangentZ.Vector.W = 255;
}

FVoxelDynamicMeshVertex::FVoxelDynamicMeshVertex(const FVector& InPosition, const FVector& InTangentX, const FVector& InTangentZ, const FVector2D& InTexCoord1,const FVector2D& InTexCoord2,const FVector2D& InTexCoord3, const FColor& InColor) :
	Position(InPosition),
	TextureCoordinate(InTexCoord1),
	TangentX(InTangentX),
	TangentZ(InTangentZ),
	Color(InColor)
{
	// basis determinant default to +1.0
	TangentZ.Vector.W = 255;
}

void FVoxelDynamicMeshVertex::SetTangents(const FVector& InTangentX, const FVector& InTangentY, const FVector& InTangentZ)
{
	TangentX = InTangentX;
	TangentZ = InTangentZ;
	// store determinant of basis in w component of normal vector
	TangentZ.Vector.W = GetBasisDeterminantSign(InTangentX, InTangentY, InTangentZ) < 0.0f ? 0 : 255;
}

FVector FVoxelDynamicMeshVertex::GetTangentY()
{
	FVector TanX = TangentX;
	FVector TanZ = TangentZ;

	return (TanZ ^ TanX) * ((float)TangentZ.Vector.W / 127.5f - 1.0f);
}

///////////////////////////////////////////////////////////////////////////


void FVoxelProcMeshVertexBuffer::InitRHI()
{
	const uint32 SizeInBytes = Vertices.Num() * sizeof(FVoxelDynamicMeshVertex);

	FVoxelProcMeshVertexResourceArray ResourceArray(Vertices.GetData(), SizeInBytes);
	FRHIResourceCreateInfo CreateInfo(&ResourceArray);
	VertexBufferRHI = RHICreateVertexBuffer(SizeInBytes, BUF_Static, CreateInfo);
}


//////////////////////////////////////////////////////////////////////////


void FVoxelProcMeshIndexBuffer::InitRHI()
{
	FRHIResourceCreateInfo CreateInfo;
	void* Buffer = nullptr;
	IndexBufferRHI = RHICreateAndLockIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), BUF_Static, CreateInfo, Buffer);

	// Write the indices to the index buffer.		
	FMemory::Memcpy(Buffer, Indices.GetData(), Indices.Num() * sizeof(int32));
	RHIUnlockIndexBuffer(IndexBufferRHI);
}


//////////////////////////////////////////////////////////////////////////


void FVoxelVertexFactoryShaderParameters::SetMesh(FRHICommandList& RHICmdList, FShader* Shader, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, uint32 DataFlags) const
{
	if (BatchElement.bUserDataIsColorVertexBuffer)
	{
		FColorVertexBuffer* OverrideColorVertexBuffer = (FColorVertexBuffer*)BatchElement.UserData;
		check(OverrideColorVertexBuffer);
		static_cast<const FVoxelVertexFactory*>(VertexFactory)->SetColorOverrideStream(RHICmdList, OverrideColorVertexBuffer);
	}
}


//////////////////////////////////////////////////////////////////////////


#if ENGINE_MINOR_VERSION < 19
FVoxelVertexFactory::FVoxelVertexFactory()
	: ColorStreamIndex(-1)
{
}
#else
FVoxelVertexFactory::FVoxelVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
	: ColorStreamIndex(-1)
	, FVertexFactory(InFeatureLevel)
{
}
#endif

void FVoxelVertexFactory::InitRHI()
{
	// If the vertex buffer containing position is not the same vertex buffer containing the rest of the data,
	// then initialize PositionStream and PositionDeclaration.
	if (Data.PositionComponent.VertexBuffer != Data.TangentBasisComponents[0].VertexBuffer)
	{
		FVertexDeclarationElementList PositionOnlyStreamElements;
		PositionOnlyStreamElements.Add(AccessPositionStreamComponent(Data.PositionComponent, 0));
		InitPositionDeclaration(PositionOnlyStreamElements);
	}

	FVertexDeclarationElementList Elements;
	if (Data.PositionComponent.VertexBuffer != NULL)
	{
		Elements.Add(AccessStreamComponent(Data.PositionComponent, 0));
	}

	// only tangent,normal are used by the stream. the binormal is derived in the shader
	uint8 TangentBasisAttributes[2] = { 1, 2 };
	for (int32 AxisIndex = 0; AxisIndex < 2; AxisIndex++)
	{
		if (Data.TangentBasisComponents[AxisIndex].VertexBuffer != NULL)
		{
			Elements.Add(AccessStreamComponent(Data.TangentBasisComponents[AxisIndex], TangentBasisAttributes[AxisIndex]));
		}
	}

	if (Data.ColorComponent.VertexBuffer)
	{
		Elements.Add(AccessStreamComponent(Data.ColorComponent, 3));
	}
	else
	{
		//If the mesh has no color component, set the null color buffer on a new stream with a stride of 0.
		//This wastes 4 bytes of bandwidth per vertex, but prevents having to compile out twice the number of vertex factories.
		FVertexStreamComponent NullColorComponent(&GNullColorVertexBuffer, 0, 0, VET_Color);
		Elements.Add(AccessStreamComponent(NullColorComponent, 3));
	}
	ColorStreamIndex = Elements.Last().StreamIndex;

	if (Data.TextureCoordinates.Num())
	{
		const int32 BaseTexCoordAttribute = 4;
		for (int32 CoordinateIndex = 0; CoordinateIndex < Data.TextureCoordinates.Num(); CoordinateIndex++)
		{
			Elements.Add(AccessStreamComponent(
				Data.TextureCoordinates[CoordinateIndex],
				BaseTexCoordAttribute + CoordinateIndex
			));
		}

		for (int32 CoordinateIndex = Data.TextureCoordinates.Num(); CoordinateIndex < MAX_STATIC_TEXCOORDS / 2; CoordinateIndex++)
		{
			Elements.Add(AccessStreamComponent(
				Data.TextureCoordinates[Data.TextureCoordinates.Num() - 1],
				BaseTexCoordAttribute + CoordinateIndex
			));
		}
	}

	if (Data.LightMapCoordinateComponent.VertexBuffer)
	{
		Elements.Add(AccessStreamComponent(Data.LightMapCoordinateComponent, 15));
	}
	else if (Data.TextureCoordinates.Num())
	{
		Elements.Add(AccessStreamComponent(Data.TextureCoordinates[0], 15));
	}

	check(Streams.Num() > 0);

	InitDeclaration(Elements);

	check(IsValidRef(GetDeclaration()));
}

void FVoxelVertexFactory::Init_RenderThread(const FVoxelProcMeshVertexBuffer* VertexBuffer)
{
	check(IsInRenderingThread());

	// Initialize the vertex factory's stream components.
	FDataType NewData;
	NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FVoxelDynamicMeshVertex, Position, VET_Float3);
	NewData.TextureCoordinates.Add(FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FVoxelDynamicMeshVertex, TextureCoordinate), sizeof(FVoxelDynamicMeshVertex), VET_Float2));
	NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FVoxelDynamicMeshVertex, TangentX, VET_PackedNormal);
	NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FVoxelDynamicMeshVertex, TangentZ, VET_PackedNormal);
	NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FVoxelDynamicMeshVertex, Color, VET_Color);
	SetData(NewData);
}

void FVoxelVertexFactory::Init(const FVoxelProcMeshVertexBuffer* VertexBuffer)
{
	if (IsInRenderingThread())
	{
		Init_RenderThread(VertexBuffer);
	}
	else
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			InitVoxelVertexFactory,
			FVoxelVertexFactory*, VertexFactory, this,
			const FVoxelProcMeshVertexBuffer*, VertexBuffer, VertexBuffer,
			{
				VertexFactory->Init_RenderThread(VertexBuffer);
			});
	}
}

void FVoxelVertexFactory::SetData(const FDataType& InData)
{
	check(IsInRenderingThread());

	// The shader code makes assumptions that the color component is a FColor, performing swizzles on ES2 and Metal platforms as necessary
	// If the color is sent down as anything other than VET_Color then you'll get an undesired swizzle on those platforms
	check((InData.ColorComponent.Type == VET_None) || (InData.ColorComponent.Type == VET_Color));

	Data = InData;
	UpdateRHI();
}

#if ENGINE_MINOR_VERSION < 19
bool FVoxelVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return true;
}
#else
bool FVoxelVertexFactory::ShouldCompilePermutation(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return true; 
}
#endif

void FVoxelVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment)
{

}

bool FVoxelVertexFactory::SupportsTessellationShaders()
{
	return true;
}

FVertexFactoryShaderParameters* FVoxelVertexFactory::ConstructShaderParameters(EShaderFrequency ShaderFrequency)
{
	if (ShaderFrequency == SF_Vertex)
	{
		return new FVoxelVertexFactoryShaderParameters();
	}

	return NULL;
}

FORCEINLINE_DEBUGGABLE void FVoxelVertexFactory::SetColorOverrideStream(FRHICommandList& RHICmdList, const FVertexBuffer* ColorVertexBuffer) const
{
	checkf(ColorVertexBuffer->IsInitialized(), TEXT("Color Vertex buffer was not initialized! Name %s"), *ColorVertexBuffer->GetFriendlyName());
#if ENGINE_MINOR_VERSION < 19
	checkf(IsInitialized() && Data.ColorComponent.bSetByVertexFactoryInSetMesh && ColorStreamIndex > 0, TEXT("Per-mesh colors with bad stream setup! Name %s"), *ColorVertexBuffer->GetFriendlyName());
#else	
	checkf(IsInitialized() && EnumHasAnyFlags(EVertexStreamUsage::Overridden, Data.ColorComponent.VertexStreamUsage) && ColorStreamIndex > 0, TEXT("Per-mesh colors with bad stream setup! Name %s"), *ColorVertexBuffer->GetFriendlyName());
#endif
	RHICmdList.SetStreamSource(ColorStreamIndex, ColorVertexBuffer->VertexBufferRHI, 0);
}

IMPLEMENT_VERTEX_FACTORY_TYPE(FVoxelVertexFactory, "/Plugin/Voxel/Private/VoxelVertexFactory.ush", true, true, true, true, true);