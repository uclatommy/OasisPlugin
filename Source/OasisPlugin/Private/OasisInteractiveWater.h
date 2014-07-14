#pragma once
#include "OasisInteractiveWater.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(OasisLog, Log, All);

UCLASS(Blueprintable)
class AOasisInteractiveWater : public AActor
{
	GENERATED_UCLASS_BODY()

	//private member variables
	int SizeX, SizeY;									//array dimensions for height field
	TArray<uint8> textureData;							//array of height data and normals
	TArray<float> m_uv;									//height and velocity data
	UTexture2D *OasisWaterTexture;						//height data stored as texture: RGB->Normals, Alpha->Height
	UMaterialInterface *MasterMaterialRef;				//reference to parameterized material template
	UMaterialInstanceDynamic* WaterMaterialInstance;	//reference to parameterized material instance that will be used to manipulate vertex locations and normal map
	

	//private member functions
	virtual void Tick(float DeltaSeconds) override;
	int VelocityAt(int u, int v);						//returns the index value used to retrieve Velocity from m_uv
	int HeightAt(int u, int v);							//returns the index value used to retrieve Height from m_uv

public:

	UFUNCTION(BlueprintCallable, Category = "Oasis")
		void Simulate(float DeltaSeconds);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Oasis")
		TSubobjectPtr<UStaticMeshComponent> SurfaceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Oasis")
		float DampingFactor;

	UPROPERTY(BlueprintReadWrite, Category = "Oasis")
		bool textureNeedsUpdate;							//tells us when to redraw the texture

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Oasis")
	//	TSubobjectPtr<USphereComponent> BaseCollisionComponent;
};