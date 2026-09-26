#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ASPlayerController.generated.h"

class AASPlayerCharacter;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * Twin-stick style controls: WASD / left stick to move, mouse / right stick to aim, space / A to dash.
 * Input actions and the mapping context are created in code, so no input assets are needed.
 */
UCLASS()
class ARENASURVIVOR_API AASPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AASPlayerController();

	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void BuildInputAssets();
	void UpdateAim();
	AASPlayerCharacter* GetLivingCharacter() const;

	void HandleMove(const FInputActionValue& Value);
	void HandleMoveReleased(const FInputActionValue& Value);
	void HandleAim(const FInputActionValue& Value);
	void HandleAimReleased(const FInputActionValue& Value);
	void HandleFirePressed();
	void HandleFireReleased();
	void HandleDash();
	void HandleRestart();

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> DefaultMapping;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> RestartAction;

	FVector2D MoveInput = FVector2D::ZeroVector;
	FVector2D StickAim = FVector2D::ZeroVector;
};
