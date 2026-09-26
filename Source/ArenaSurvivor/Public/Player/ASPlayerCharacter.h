#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ASPlayerCharacter.generated.h"

class UASHealthComponent;
class UASWeaponComponent;
class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;

/** Top-down player. Faces the controller's aim, fires from its weapon component. */
UCLASS()
class ARENASURVIVOR_API AASPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AASPlayerCharacter();

	UASHealthComponent* GetHealthComponent() const { return HealthComponent; }
	UASWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

	UFUNCTION(BlueprintPure, Category = "Player")
	bool IsAlive() const;

	/** Short burst of speed with brief invulnerability. Returns false while on cooldown. */
	UFUNCTION(BlueprintCallable, Category = "Player")
	bool TryDash(FVector Direction);

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetDashCooldownRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetDashCooldown() const { return DashCooldown; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	/** Small block on the front of the body so the aim direction is readable. */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> NoseMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UASHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UASWeaponComponent> WeaponComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	FLinearColor BodyColor = FLinearColor(0.1f, 0.55f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashSpeed = 2200.f;

	/** Small upward kick so the dash is carried through the air instead of eaten by ground friction. */
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashLift = 160.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashCooldown = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashInvulnerabilityTime = 0.3f;

private:
	void EndDashInvulnerability();

	FTimerHandle DashInvulnerabilityTimer;
	float LastDashTime = -1000.f;

	UFUNCTION()
	void HandleDeath(UASHealthComponent* DeadComponent, AController* Killer);
};
