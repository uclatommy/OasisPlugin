#include "OasisPluginPrivatePCH.h"
#include "OasisInteractiveWater.h"


AOasisInteractiveWater::AOasisInteractiveWater(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	BaseCollisionComponent = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("BaseSphereComponent"));
	RootComponent = BaseCollisionComponent;
	SurfaceMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("SurfaceMesh"));
	SurfaceMesh->SetSimulatePhysics(false);
	SurfaceMesh->AttachTo(RootComponent);
	//WaterMaterialInstance = UMaterialInstanceDynamic::Create(SurfaceMesh->GetMaterial(0), this);
	PrimaryActorTick.bCanEverTick = true;
	
	// create random noise for texture
	const int SizeX = 512;
	const int SizeY = 512;
	static uint8 data[SizeX * SizeY * 4] = { 0 };
	srand(1982);
	for (int i = 0; i < SizeX * SizeY * 4; ++i) {
		data[i] = rand() % 255;
	}

	OasisWaterTexture = UTexture2D::CreateTransient(SizeX, SizeY);
	OasisWaterTexture->AddToRoot();
	OasisWaterTexture->UpdateResource();

	FUpdateTextureRegion2D *r;
	r = new FUpdateTextureRegion2D(0, 0, 0, 0, SizeX, SizeY);
	UpdateTextureRegions(OasisWaterTexture, (int32)0, (uint32)1, r, (uint32)(4 * SizeX), (uint32)4, data, false); // Using the UpdateTextureRegions from https://wiki.unrealengine.com/Dynamic_Textures
	OasisWaterTexture->UpdateResource();

	// update texture property on material and set object to use material
	//WaterMaterialInstance->SetTextureParameterValue(FName("SurfaceDataTexture"), OasisWaterTexture);

	//SurfaceMesh->SetMaterial(0, WaterMaterialInstance);
}

void AOasisInteractiveWater::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
			for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
			{
				int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
				if (RegionData->MipIndex >= CurrentFirstMip)
				{
					RHIUpdateTexture2D(
						RegionData->Texture2DResource->GetTexture2DRHI(),
						RegionData->MipIndex - CurrentFirstMip,
						RegionData->Regions[RegionIndex],
						RegionData->SrcPitch,
						RegionData->SrcData
						+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
						+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
				}
			}
			if (bFreeData)
			{
				FMemory::Free(RegionData->Regions);
				FMemory::Free(RegionData->SrcData);
			}
			delete RegionData;
			});
	}
}

void AOasisInteractiveWater::Simulate()
{

	//OasisWaterTexture->UpdateResource();
	/*
	const FColor ColorOne = FColor(0, 255, 0);
	const FColor ColorTwo = FColor(255, 0, 0);
	const int32 CheckerSize = 512;
	const int32 HalfPixelNum = CheckerSize >> 1;

	FColor* MipData = static_cast<FColor*>(OasisWaterTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	
	for (int32 RowNum = 0; RowNum < CheckerSize; ++RowNum)
	{
		for (int32 ColNum = 0; ColNum < CheckerSize; ++ColNum)
		{
			FColor& CurColor = MipData[(ColNum + (RowNum * CheckerSize))];

			if (ColNum < HalfPixelNum)
			{
				CurColor = ColorOne;
			}
			else
			{
				CurColor = ColorTwo;
			}
		}
	}
	OasisWaterTexture->PlatformData->Mips[0].BulkData.Unlock();
	OasisWaterTexture->UpdateResource();
	*/
}