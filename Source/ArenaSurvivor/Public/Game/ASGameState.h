#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ASGameState.generated.h"

/** Match progress read by the HUD. Written only by AASGameMode. */
UCLASS()
class ARENASURVIVOR_API AASGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	int32 Wave = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	int32 Kills = 0;

	/** Enemies still to spawn plus enemies alive in the current wave. */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	int32 EnemiesRemaining = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	bool bIntermission = true;

	/** World time at which the next wave begins. Only meaningful during intermission. */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	float NextWaveTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	bool bGameOver = false;

	/** Best score from previous runs, loaded from the save slot at match start. */
	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	int32 HighScore = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	int32 BestWave = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Arena")
	bool bNewHighScore = false;
};
