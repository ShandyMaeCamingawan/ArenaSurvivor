#pragma once

#include "CoreMinimal.h"
#include "Enemies/ASEnemyTypes.h"
#include "GameFramework/GameModeBase.h"
#include "ASGameMode.generated.h"

class AASEnemyCharacter;
class AASGameState;
class AASPickup;
class AStaticMeshActor;
class UASHealthComponent;
class UMaterialInterface;
class UStaticMesh;

/**
 * Builds the arena at runtime and runs the waves: intermission, staggered
 * spawning, then the next wave once every enemy is dead.
 */
UCLASS()
class ARENASURVIVOR_API AASGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AASGameMode();

	virtual void StartPlay() override;
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	/** Pure wave-tuning functions, kept static so they can be unit tested. */
	static int32 GetEnemyCountForWave(int32 Wave);
	static float GetEnemyHealthMultiplierForWave(int32 Wave);
	/** Roll is a uniform random number in [0, 1). */
	static EASEnemyType PickEnemyTypeForWave(int32 Wave, float Roll);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	float ArenaHalfExtent = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	float WallHeight = 250.f;

	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	float WallThickness = 100.f;

	/** Pawns spawn at the origin, so the floor sits just below capsule height. */
	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	float FloorTopZ = -100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Waves")
	float FirstWaveDelay = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Waves")
	float IntermissionTime = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "Waves")
	float SpawnInterval = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category = "Waves")
	int32 MaxAliveEnemies = 35;

	UPROPERTY(EditDefaultsOnly, Category = "Waves")
	float MinSpawnDistanceFromPlayer = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<AASEnemyCharacter> EnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<AASPickup> PickupClass;

	UPROPERTY(EditDefaultsOnly, Category = "Pickups", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RapidFireDropShare = 0.3f;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ShapeMaterial;

private:
	void BuildArena();
	void SpawnLights();
	AStaticMeshActor* SpawnBlock(const FVector& Center, const FVector& Size, const FLinearColor& Color);

	void ScheduleNextWave(float Delay);
	void StartNextWave();
	void SpawnTick();
	bool FindSpawnLocation(FVector& OutLocation) const;
	void SpawnEnemy(const FVector& Location, EASEnemyType Type);
	void TrySpawnPickup(const FVector& Location, float Chance);
	void RefreshEnemiesRemaining();

	UFUNCTION()
	void HandleEnemyDeath(UASHealthComponent* HealthComponent, AController* Killer);

	UFUNCTION()
	void HandlePlayerDeath(UASHealthComponent* HealthComponent, AController* Killer);

	AASGameState* GetArenaState() const;

	FTimerHandle WaveTimer;
	FTimerHandle SpawnTimer;
	int32 PendingSpawns = 0;
	int32 AliveEnemies = 0;
};
