#include "OasisPluginPrivatePCH.h"
#include "OasisInteractiveWater.h"

DEFINE_LOG_CATEGORY(OasisLog);

AOasisInteractiveWater::AOasisInteractiveWater(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP), SizeX(64), SizeY(64)
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

	for (int j = 0; j < SizeY; j++)
	{
		for (int i = 0; i < SizeX; i++)
		{
			m_uv.Add(0);			//this element will hold height at pixel (i,j)
			m_uv.Add(0);			//this element will hold velocity at pixel (i,j)
			m_gradients.Add(0);		//this element will hold dz/dx for (i,j)
			m_gradients.Add(0);		//this element will hold dz/dy for (i,j)
		}
	}

	OasisWaterTexture = UTexture2D::CreateTransient(SizeX, SizeY, PF_B8G8R8A8);
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
		for (int j = 0; j < SizeY; j++)
		{
			for (int i = 0; i < SizeX; i++)
			{
				MipData[i + j*SizeX] = FColor((uint8)(m_uv[HeightAt(i, j)] * 255), 0, 0, 255);
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
	for (int j = 1; j<SizeY - 1; j++) 
	{
		for (int i = 1; i<SizeX - 1; i++)
		{
			m_uv[VelocityAt(i,j)] += DeltaSeconds * (m_uv[HeightAt(i - 1, j)] + m_uv[HeightAt(i + 1, j)] + m_uv[HeightAt(i, j - 1)] + m_uv[HeightAt(i, j + 1)])*0.25f - m_uv[HeightAt(i, j)];
		}
	}

	for (int j = 1; j<SizeY - 1; j++)
	{
		for (int i = 1; i<SizeX - 1; i++)
		{
			float &v = m_uv[VelocityAt(i, j)];
			v *= DampingFactor;
			m_uv[HeightAt(i, j)] += DeltaSeconds*v;
			//m_uv[HeightAt(i, j)] += 0.01;
		}
	}
	
	CalculateGradients();
	
	textureNeedsUpdate = true;
}

void AOasisInteractiveWater::CalculateGradients()
{
	for (int j = 1; j<SizeY - 1; j++)
	{
		for (int i = 1; i<SizeX - 1; i++)
		{
			m_gradients[ddxAt(i,j)] = (m_uv[HeightAt(i + 1, j)] - m_uv[HeightAt(i - 1, j)]);		//dz/dx
			m_gradients[ddyAt(i,j)] = (m_uv[HeightAt(i, j + 1)] - m_uv[HeightAt(i, j - 1)]);		//dz/dy
		}
	}
}

void AOasisInteractiveWater::addDisturbance(float x, float y, float r, float s)
{
	int ix = (int)floorf(x);
	int iy = (int)floorf(y);
	int ir = (int)ceilf(r);
	int sx = FMath::Max<int>(1, ix - ir);
	int sy = FMath::Max<int>(1, iy - ir);
	int ex = FMath::Min<int>(ix + ir, SizeX - 2);
	int ey = FMath::Min<int>(iy + ir, SizeY - 2);
	for (int j = sy; j <= ey; j++) {
		for (int i = sx; i <= ex; i++) {
			float dx = x - i;
			float dy = y - j;
			float d = sqrtf(dx*dx + dy*dy);
			if (d < r) {
				d = (d / r)*3.0f;
				m_uv[HeightAt(i, j)] += expf(-d*d)*s;
			}
		}
	}
}

//index mapping helper functions
int AOasisInteractiveWater::VelocityAt(int u, int v) //for m_uv indices
{
	return (u + v*SizeX)*2 + 1;
}

int AOasisInteractiveWater::HeightAt(int u, int v) //for m_uv indices
{
	return (u + v*SizeX)*2;
}

int AOasisInteractiveWater::ddxAt(int u, int v) //for m_gradients indices
{
	return HeightAt(u, v);
}

int AOasisInteractiveWater::ddyAt(int u, int v) //for m_gradient indices
{
	return VelocityAt(u, v);
}