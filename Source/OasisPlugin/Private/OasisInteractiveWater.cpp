#include "OasisPluginPrivatePCH.h"
#include "OasisInteractiveWater.h"

DEFINE_LOG_CATEGORY(OasisLog);

AOasisInteractiveWater::AOasisInteractiveWater(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP), SizeX(96), SizeY(96)
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
	WaterHeight = 1.0f;

	RootComponent = SurfaceMesh;

}

void AOasisInteractiveWater::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	WaterMaterialInstance->SetVectorParameterValue(FName(TEXT("Color")), SurfaceColor);
	SurfaceMesh->SetMaterial(0, WaterMaterialInstance);
	ResetOasisTexture();
	this->GetActorBounds(false, meshOrigin, meshExtent);
}

void AOasisInteractiveWater::ResetOasisTexture()
{
	TArray<float> new_uv;
	TArray<float> new_gradients;
	for (int j = 0; j < SizeY; j++)
	{
		for (int i = 0; i < SizeX; i++)
		{
			new_uv.Add(0);				//this element will hold height at pixel (i,j)
			new_uv.Add(0);				//this element will hold velocity at pixel (i,j)
			new_gradients.Add(0);		//this element will hold dz/dx for (i,j)
			new_gradients.Add(0);		//this element will hold dz/dy for (i,j)
		}
	}
	m_uv = new_uv;
	m_gradients = new_gradients;
	
	OasisWaterTexture = UTexture2D::CreateTransient(SizeX, SizeY, PF_B8G8R8A8);
	OasisWaterTexture->AddToRoot();
	OasisWaterTexture->UpdateResource();

	textureNeedsUpdate = true;
}

void AOasisInteractiveWater::SetGroundHeight(const UTexture2D *GroundHeightTexture)
{
	FColor *MipData = static_cast<FColor*>(GroundHeightTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
	for (int j = 0; j < SizeY; j++)
	{
		for (int i = 0; i < SizeX; i++)
		{
			GroundHeight.Add(MipData[GroundHeightAt(i, j)].R/255.0f);
		}
	}
	GroundHeightTexture->PlatformData->Mips[0].BulkData.Unlock();
}

void AOasisInteractiveWater::Tick(float DeltaSeconds)
{
	if (textureNeedsUpdate)
	{
		FColor *MipData = static_cast<FColor*>(OasisWaterTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
		float DDX, DDY, c;
		for (int j = 0; j < SizeY; j++)
		{
			for (int i = 0; i < SizeX; i++)
			{
				DDX = m_gradients[ddxAt(i, j)];
				DDY = m_gradients[ddyAt(i, j)];
				c = 1.0f / (sqrtf(DDX*DDX + DDY*DDY + 1.0f));
				MipData[i + j*SizeX] = FColor((uint8)(((-c * DDX) + 1.0f) / 2.0f * 255), (uint8)(((-c * DDY) + 1.0f) / 2.0f * 255), (uint8)(c*255), (uint8)(m_uv[HeightAt(i, j)] * 255));
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
void AOasisInteractiveWater::Simulate(float TimeFactor)
{	
	//generate a height field
	for (int j = 1; j<SizeY - 1; j++) 
	{
		for (int i = 1; i<SizeX - 1; i++)
		{ 
//			if (GroundHeight[GroundHeightAt(i,j)]<WaterHeight)
//			{
				m_uv[VelocityAt(i, j)] += TimeFactor * (m_uv[HeightAt(i - 1, j)] + m_uv[HeightAt(i + 1, j)] + m_uv[HeightAt(i, j - 1)] + m_uv[HeightAt(i, j + 1)])*0.25f - m_uv[HeightAt(i, j)];
//			}
/*			else 
			{
				m_uv[VelocityAt(i, j)] = 0.0f;
			}
*/		}
	}

	for (int j = 1; j<SizeY - 1; j++)
	{
		for (int i = 1; i<SizeX - 1; i++)
		{
//			if (GroundHeight[GroundHeightAt(i, j)]<WaterHeight)
//			{
				float &v = m_uv[VelocityAt(i, j)];
				v *= DampingFactor;
				m_uv[HeightAt(i, j)] += TimeFactor*v;
//			}
//			else
//			{
//				m_uv[HeightAt(i, j)] = GroundHeight[GroundHeightAt(i, j)];
//			}
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

void AOasisInteractiveWater::addDisturbance(float x, float y, float radius, float strength)
{
	int ix = (int)floorf(x);
	int iy = (int)floorf(y);
	int ir = (int)ceilf(radius);
	int sx = FMath::Max<int>(1, ix - ir);
	int sy = FMath::Max<int>(1, iy - ir);
	int ex = FMath::Min<int>(ix + ir, SizeX - 2);
	int ey = FMath::Min<int>(iy + ir, SizeY - 2);
	for (int j = sy; j <= ey; j++) {
		for (int i = sx; i <= ex; i++) {
			float dx = x - i;
			float dy = y - j;
			float d = sqrtf(dx*dx + dy*dy);
			if (d < radius) {
				d = (d / radius)*3.0f;
				m_uv[HeightAt(i, j)] += expf(-d*d)*(strength);
			}
		}
	}
}

TArray<float> AOasisInteractiveWater::DistanceOfActorToThisMeshSurface(TArray<AActor*> TargetActors, TArray<FVector> &ClosestSurfacePoints) const
{
	//Dist of Actor to Surface, retrieve closest Surface Point to Actor
	ClosestSurfacePoints.Empty();
	TArray<float> Distances;
	for (auto it(TargetActors.CreateIterator()); it; ++it)
	{
		if (!(*it)->IsValidLowLevel()) continue;
		ClosestSurfacePoints.Add(FVector(0.0f, 0.0f, 0.0f));
		Distances.Add(SurfaceMesh->GetDistanceToCollision((*it)->GetActorLocation(), ClosestSurfacePoints.Last()));
	}
	return Distances;
}

void AOasisInteractiveWater::setGridDimensions(int32 sizeX, int32 sizeY)
{
	SizeX = sizeX;
	SizeY = sizeY;
	ResetOasisTexture();
}

void AOasisInteractiveWater::WS2Texture(float InXWS, float InYWS, float &outXTS, float &outYTS)
{
	//convert coordinate system to lower left corner as origin
	float OriginX = meshOrigin.Component(0) - meshExtent.Component(0);
	float OriginY = meshOrigin.Component(1) - meshExtent.Component(1);

	//figure out relative location of the hit as a ratio for x and y
	float hitX = (InXWS - OriginX) / (2 * meshExtent.Component(0));
	float hitY = (InYWS - OriginY) / (2 * meshExtent.Component(1));

	//use the ratio to convert the hit to texture coordinates
	outXTS = hitX * SizeX;
	outYTS = hitY * SizeY;
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

int AOasisInteractiveWater::GroundHeightAt(int u, int v)
{
	return u + v*SizeX;
}