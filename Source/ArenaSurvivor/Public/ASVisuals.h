#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Everything in the game is drawn with the engine's basic shapes and one tintable
 * material, so the project runs without any imported art.
 */
namespace ASVisuals
{
	inline const TCHAR* CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	inline const TCHAR* SpherePath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	inline const TCHAR* CylinderPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
	inline const TCHAR* ConePath = TEXT("/Engine/BasicShapes/Cone.Cone");

	/** Only valid inside a UObject constructor. */
	ARENASURVIVOR_API UStaticMesh* FindShape(const TCHAR* Path);

	/** Only valid inside a UObject constructor. */
	ARENASURVIVOR_API UMaterialInterface* FindShapeMaterial();

	/** Tints the mesh through a dynamic instance of BasicShapeMaterial's "Color" parameter. */
	ARENASURVIVOR_API void SetColor(UStaticMeshComponent* Mesh, const FLinearColor& Color);
}
