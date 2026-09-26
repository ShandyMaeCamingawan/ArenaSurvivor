#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ASWeaponComponent.generated.h"

class AASProjectile;

/** Automatic projectile weapon. Fires along the owner's facing while the trigger is held. */
UCLASS(ClassGroup = (ArenaSurvivor), meta = (BlueprintSpawnableComponent))
class ARENASURVIVOR_API UASWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UASWeaponComponent();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

	/** Temporarily multiplies the fire rate. A new boost replaces the current one. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ApplyFireRateBoost(float Multiplier, float Duration);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetBoostTimeRemaining() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AASProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "0.1"))
	float ShotsPerSecond = 7.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float Damage = 25.f;

	/** Random yaw offset, in degrees either side of the aim direction. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float SpreadDegrees = 2.5f;

	/** Spawn offset relative to the owner's transform. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FVector MuzzleOffset = FVector(80.f, 0.f, 20.f);

private:
	void Fire();
	void RestartFireTimer();
	void ClearBoost();
	float GetFireInterval() const { return 1.f / (ShotsPerSecond * FireRateMultiplier); }

	FTimerHandle FireTimer;
	FTimerHandle BoostTimer;
	float LastFireTime = -1000.f;
	float FireRateMultiplier = 1.f;
	bool bWantsToFire = false;
};
