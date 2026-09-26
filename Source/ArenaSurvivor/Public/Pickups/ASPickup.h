#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASPickup.generated.h"

class AASPlayerCharacter;
class URotatingMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EASPickupType : uint8
{
	Health,
	RapidFire
};

/** Spinning collectible dropped by enemies. Despawns if left alone. */
UCLASS()
class ARENASURVIVOR_API AASPickup : public AActor
{
	GENERATED_BODY()

public:
	AASPickup();

	/** Call before BeginPlay (e.g. between SpawnActorDeferred and FinishSpawning). */
	void SetPickupType(EASPickupType NewType) { PickupType = NewType; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> Trigger;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<URotatingMovementComponent> Rotator;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	EASPickupType PickupType = EASPickupType::Health;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float HealAmount = 35.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float RapidFireMultiplier = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float RapidFireDuration = 6.f;

private:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Returns false if the pickup would do nothing (e.g. healing at full health). */
	bool ApplyTo(AASPlayerCharacter* PlayerCharacter);
};
