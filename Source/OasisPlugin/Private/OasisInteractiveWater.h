#pragma once
#include "OasisInteractiveWater.generated.h"

UCLASS(Blueprintable)
class AOasisInteractiveWater : public AActor
{
	GENERATED_UCLASS_BODY()

	void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);

public:

	UPROPERTY(BluePrintReadOnly, Category = "Oasis")
		UTexture2D *OasisWaterTexture;

	UFUNCTION(BlueprintCallable, Category = "Oasis")
		void Simulate();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Oasis")
		TSubobjectPtr<UStaticMeshComponent> SurfaceMesh;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Oasis")
		TSubobjectPtr<USphereComponent> BaseCollisionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Oasis")
		UMaterialInterface *MasterMaterialRef;

	UPROPERTY(BluePrintReadOnly, Category = "Oasis")
		UMaterialInstanceDynamic* WaterMaterialInstance;
};