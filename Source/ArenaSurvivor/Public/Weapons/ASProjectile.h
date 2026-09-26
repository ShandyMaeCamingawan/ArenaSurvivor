#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ASProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ARENASURVIVOR_API AASProjectile : public AActor
{
	GENERATED_BODY()

public:
	AASProjectile();

	void SetDamage(float NewDamage) { Damage = NewDamage; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float Damage = 25.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	FLinearColor Color = FLinearColor(1.f, 0.85f, 0.2f);

private:
	UFUNCTION()
	void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
