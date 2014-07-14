#include "OasisPluginPrivatePCH.h"
#include "OasisInteractiveWater.h"

DEFINE_LOG_CATEGORY(OasisLog);

AOasisInteractiveWater::AOasisInteractiveWater(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP), SizeX(512), SizeY(512)
{
	//BaseCollisionComponent = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("BaseSphereComponent"));
	//RootComponent = BaseCollisionComponent;
	SurfaceMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("SurfaceMesh"));
	SurfaceMesh->SetSimulatePhysics(false);
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> waterMesh(TEXT("StaticMesh'/Game/Shapes/Shape_Plane.Shape_Plane'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> waterMaterial(TEXT("Material'/Game/Materials/M_Interactive_Water.M_Interactive_Water'"));

	MasterMaterialRef = waterMaterial.Object;
	SurfaceMesh->SetStaticMesh(waterMesh.Object);
	WaterMaterialInstance = UMaterialInstanceDynamic::Create(MasterMaterialRef, this);

	RootComponent = SurfaceMesh;

	for (int i = 0; i < SizeX * SizeY * 4; ++i) {
		switch (i % 4)
		{
		case 0: //R channel
			textureData.Add((uint8)0);
			break;
		case 1: //G channel
			textureData.Add((uint8)0);
			break;
		case 2: //B channel
			textureData.Add((uint8)0);
			break;
		case 3: //Alpha
			textureData.Add((uint8)255);
			break;
		}
	}

	OasisWaterTexture = UTexture2D::CreateTransient(SizeX, SizeY, PF_R8G8B8A8);
	OasisWaterTexture->AddToRoot();
	OasisWaterTexture->UpdateResource();
	textureNeedsUpdate = true;
	DampingFactor = 0.5f;
}

void AOasisInteractiveWater::Tick(float DeltaSeconds)
{
	if (textureNeedsUpdate)
	{
		FColor *MipData = static_cast<FColor*>(OasisWaterTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
		int pixelNum = 0;
		for (int32 r = 0; r < SizeY; ++r)
		{
			for (int32 c = 0; c < SizeX; ++c)
			{
				pixelNum = c + (r*SizeX);
				MipData[pixelNum] = FColor(textureData[pixelNum*4], textureData[pixelNum*4 + 1], textureData[pixelNum*4 + 2], textureData[pixelNum*4 + 3]);
			}
		}
		OasisWaterTexture->PlatformData->Mips[0].BulkData.Unlock();
		OasisWaterTexture->UpdateResourceW();
		WaterMaterialInstance->SetTextureParameterValue(FName("T2DParam"), OasisWaterTexture);
		SurfaceMesh->SetMaterial(0, WaterMaterialInstance);
		textureNeedsUpdate = false;
	}
	Super::Tick(DeltaSeconds);
}

// http://www.matthiasmueller.info/talks/GDC2008.pdf
void AOasisInteractiveWater::Simulate(float DeltaSeconds)
{	
	//generate a height field
	int arrayIndex = 0;
	for (int j = 1; j<SizeY - 1; j++) 
	{
		for (int i = 1; i<SizeX - 1; i++)
		{
			m_uv[VelocityAt(i,j)] += DeltaSeconds * (m_uv[HeightAt(i - 1, j)] + m_uv[HeightAt(i + 1, j)] + m_uv[HeightAt(i, j - 1)] + m_uv[HeightAt(i, j + 1)])*0.25f - m_uv[HeightAt(i, j)];
		}
	}

	for (int j = 1; j<SizeY - 1; j++) {
		for (int i = 1; i<SizeX - 1; i++) {
			float &v = m_uv[VelocityAt(i, j)];
			v *= DampingFactor;
			m_uv[HeightAt(i, j)] += DeltaSeconds*v;
		}
	}
	
	textureNeedsUpdate = true;
}

int AOasisInteractiveWater::VelocityAt(int u, int v)
{
	return u + v*SizeX + 1;
}

int AOasisInteractiveWater::HeightAt(int u, int v)
{
	return u + v*SizeX;
}