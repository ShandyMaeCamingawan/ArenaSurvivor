#pragma once

#include "CoreMinimal.h"
#include "Enemies/ASEnemyTypes.h"
#include "GameFramework/Character.h"
#include "ASEnemyCharacter.generated.h"

class UASHealthComponent;
class UStaticMeshComponent;

/** Melee chaser. Movement is driven by AASEnemyAIController; stats come from FASEnemyTuning. */
UCLASS()
class ARENASURVIVOR_API AASEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AASEnemyCharacter();

	/** Call between SpawnActorDeferred and FinishSpawning so the health component starts full. */
	void InitFromType(EASEnemyType InType, float HealthMultiplier);

	/** Deals contact damage to Target if the attack is off cooldown. */
	void TryAttack(AActor* Target);

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	EASEnemyType GetEnemyType() const { return EnemyType; }

	UASHealthComponent* GetHealthComponent() const { return HealthComponent; }
	const FASEnemyTuning& GetTuning() const { return Tuning; }
	float GetAttackReach() const { return AttackReach; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UASHealthComponent> HealthComponent;

	/** Extra distance beyond touching capsules at which the enemy can hit. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float AttackReach = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float AttackCooldown = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float HitFlashTime = 0.08f;

private:
	UFUNCTION()
	void HandleHealthChanged(UASHealthComponent* ChangedComponent, float NewHealth, float Delta);

	UFUNCTION()
	void HandleDeath(UASHealthComponent* DeadComponent, AController* Killer);

	void EndHitFlash();

	UPROPERTY(VisibleInstanceOnly, Category = "Enemy")
	EASEnemyType EnemyType = EASEnemyType::Grunt;

	FASEnemyTuning Tuning;
	FTimerHandle HitFlashTimer;
	float LastAttackTime = -1000.f;
};
