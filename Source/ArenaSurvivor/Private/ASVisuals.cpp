#include "ASVisuals.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FName ColorParameter(TEXT("Color"));
}

UStaticMesh* ASVisuals::FindShape(const TCHAR* Path)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(Path);
	return Finder.Object;
}

UMaterialInterface* ASVisuals::FindShapeMaterial()
{
	ConstructorHelpers::FObjectFinder<UMaterialInterface> Finder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	return Finder.Object;
}

void ASVisuals::SetColor(UStaticMeshComponent* Mesh, const FLinearColor& Color)
{
	if (!Mesh)
	{
		return;
	}

	UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0));
	if (!Material)
	{
		Material = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (Material)
	{
		Material->SetVectorParameterValue(ColorParameter, Color);
	}
}
