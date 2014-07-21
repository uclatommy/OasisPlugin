#pragma once
#include "OasisInteractiveWater.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(OasisLog, Log, All);

UCLASS(Blueprintable)
class AOasisInteractiveWater : public AActor
{
	GENERATED_UCLASS_BODY()

	//private member variables
	int32 SizeX, SizeY;									//array dimensions for height field
	TArray<float> m_uv;									//height and velocity data
	TArray<float> m_gradients;							//gradients to calc normals
	UTexture2D *OasisWaterTexture;						//height data stored as texture: RGB->Normals, Alpha->Height
	UMaterialInterface *MasterMaterialRef;				//reference to parameterized material template
	UMaterialInstanceDynamic* WaterMaterialInstance;	//reference to parameterized material instance that will be used to manipulate vertex locations and normal map	

	//private member functions
	virtual void Tick(float DeltaSeconds) override;
	int VelocityAt(int u, int v);						//returns the index value used to retrieve Velocity from m_uv
	int HeightAt(int u, int v);							//returns the index value used to retrieve Height from m_uv
	void CalculateGradients();
	int ddxAt(int u, int v);
	int ddyAt(int u, int v);
	void setOasisTexture();
	virtual void PostInitializeComponents() override;
	FVector meshOrigin, meshExtent;

public:
	//TimeFactor should be >= 1.0f
	UFUNCTION(BlueprintCallable, Category = "Oasis")
		void Simulate(float TimeFactor); 

	//creates a wave originating at x,y in texture space coordinates
	UFUNCTION(BlueprintCallable, Category = "Oasis")
		void addDisturbance(float x, float y, float r, float s);

	//sets the dimensions of the fluid texture, should be powers of 2
	UFUNCTION(BlueprintCallable, Category = "Oasis")
		void setGridDimensions(int32 sizeX, int32 sizeY);

	//determines distance of Target Actor to this mesh
	UFUNCTION(BlueprintCallable, Category = "Oasis")
		TArray<float> DistanceOfActorToThisMeshSurface(TArray<AActor*> TargetActor, TArray<FVector> &ClosestSurfacePoint) const;

	//converts world space coordinates to texture space
	UFUNCTION(BlueprintCallable, Category = "Oasis")
		void WS2Texture(float InX, float InY, float &outX, float &outY);

	//the fluid mesh
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Oasis")
		TSubobjectPtr<UStaticMeshComponent> SurfaceMesh;
	
	//determines how long a wave lasts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oasis")
		float DampingFactor;
	
	//tells us when to redraw the texture
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Oasis")
		bool textureNeedsUpdate;							
	
	//sets the surface color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oasis")
		FLinearColor SurfaceColor;
};