#include "Enemies/ASEnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "Enemies/ASEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ASPlayerCharacter.h"

AASEnemyAIController::AASEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AASEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AASEnemyCharacter* Enemy = Cast<AASEnemyCharacter>(GetPawn());
	if (!Enemy || !Enemy->IsAlive())
	{
		return;
	}

	AASPlayerCharacter* Target = Cast<AASPlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Target || !Target->IsAlive())
	{
		return;
	}

	FVector ToTarget = Target->GetActorLocation() - Enemy->GetActorLocation();
	ToTarget.Z = 0.f;
	const float Distance = ToTarget.Size();

	const float Reach = Enemy->GetCapsuleComponent()->GetScaledCapsuleRadius()
		+ Target->GetCapsuleComponent()->GetScaledCapsuleRadius()
		+ Enemy->GetAttackReach();

	if (Distance > KINDA_SMALL_NUMBER)
	{
		Enemy->AddMovementInput(ToTarget / Distance);
	}

	if (Distance <= Reach)
	{
		Enemy->TryAttack(Target);
	}
}
