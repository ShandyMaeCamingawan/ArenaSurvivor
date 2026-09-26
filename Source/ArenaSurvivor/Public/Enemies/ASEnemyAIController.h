#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ASEnemyAIController.generated.h"

/**
 * Steers straight at the player and attacks on contact. The arena is an open
 * floor, so direct steering plus RVO avoidance is enough and needs no nav mesh.
 */
UCLASS()
class ARENASURVIVOR_API AASEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AASEnemyAIController();

	virtual void Tick(float DeltaSeconds) override;
};
