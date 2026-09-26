#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ASHealthComponent.generated.h"

class AController;
class UDamageType;
class UASHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FASOnHealthChangedSignature, UASHealthComponent*, HealthComponent, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FASOnDeathSignature, UASHealthComponent*, HealthComponent, AController*, Killer);

/**
 * Hit points for any actor. Hooks the owner's OnTakeAnyDamage, so damage is dealt
 * through the regular UGameplayStatics::ApplyDamage path.
 */
UCLASS(ClassGroup = (ArenaSurvivor), meta = (BlueprintSpawnableComponent))
class ARENASURVIVOR_API UASHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UASHealthComponent();

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FASOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FASOnDeathSignature OnDeath;

	/** Restores health. Returns the amount actually healed. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float Heal(float Amount);

	/** Called before BeginPlay this sets the starting health; afterwards pass bRefill to top up. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetMaxHealth(float NewMaxHealth, bool bRefill);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetInvulnerable(bool bNewInvulnerable) { bInvulnerable = bNewInvulnerable; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthFraction() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

private:
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	void ApplyHealthDelta(float Delta, AController* Instigator);

	UPROPERTY(VisibleInstanceOnly, Category = "Health")
	float Health = 0.f;

	bool bIsDead = false;
	bool bInvulnerable = false;
};
