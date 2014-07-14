#pragma once
#include "OasisInteractiveWater.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(OasisLog, Log, All);

UCLASS(Blueprintable)
class AOasisInteractiveWater : public AActor
{
	GENERATED_UCLASS_BODY()

	//private member variables
	int SizeX, SizeY;									//array dimensions for height field
	TArray<uint8> textureData;							//array of height data
	UTexture2D *OasisWaterTexture;						//height data stored as texture: RGB->Normals, Alpha->Height
	UMaterialInterface *MasterMaterialRef;				//reference to parameterized material template
	UMaterialInstanceDynamic* WaterMaterialInstance;	//reference to parameterized material instance that will be used to manipulate vertex locations and normal map
	bool textureNeedsUpdate;							//tells us when to redraw the texture

	//private member functions
	virtual void Tick(float DeltaSeconds) override;

public:

	UFUNCTION(BlueprintCallable, Category = "Oasis")
		void Simulate(float DeltaSeconds);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Oasis")
		TSubobjectPtr<UStaticMeshComponent> SurfaceMesh;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Oasis")
	//	TSubobjectPtr<USphereComponent> BaseCollisionComponent;
};