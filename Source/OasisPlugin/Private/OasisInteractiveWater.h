#pragma once
#include "OasisInteractiveWater.generated.h"

UCLASS(Blueprintable)
class AOasisInteractiveWater : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditDefaultsOnly, Category=Materials)
	UTexture2D *DynamicTexture;

	static void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);
};