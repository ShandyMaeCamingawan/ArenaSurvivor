#include "Enemies/ASEnemyCharacter.h"
#include "ASVisuals.h"
#include "Components/ASHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Enemies/ASEnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace
{
	constexpr float BaseCapsuleRadius = 34.f;
	constexpr float BaseCapsuleHalfHeight = 88.f;
}

AASEnemyCharacter::AASEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = AASEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(BaseCapsuleRadius, BaseCapsuleHalfHeight);

	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 540.f, 0.f);
	Move->bUseRVOAvoidance = true;
	Move->AvoidanceConsiderationRadius = 200.f;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetStaticMesh(ASVisuals::FindShape(ASVisuals::CubePath));
	BodyMesh->SetMaterial(0, ASVisuals::FindShapeMaterial());
	BodyMesh->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.76f));

	HealthComponent = CreateDefaultSubobject<UASHealthComponent>(TEXT("HealthComponent"));

	Tuning = FASEnemyTuning::ForType(EnemyType);
}

void AASEnemyCharacter::InitFromType(EASEnemyType InType, float HealthMultiplier)
{
	EnemyType = InType;
	Tuning = FASEnemyTuning::ForType(InType);
	Tuning.MaxHealth *= FMath::Max(0.1f, HealthMultiplier);

	HealthComponent->SetMaxHealth(Tuning.MaxHealth, true);
	GetCharacterMovement()->MaxWalkSpeed = Tuning.MoveSpeed;

	GetCapsuleComponent()->SetCapsuleSize(BaseCapsuleRadius * Tuning.Size, BaseCapsuleHalfHeight * Tuning.Size);
	BodyMesh->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.76f) * Tuning.Size);
}

void AASEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	ASVisuals::SetColor(BodyMesh, Tuning.Color);

	HealthComponent->OnHealthChanged.AddDynamic(this, &AASEnemyCharacter::HandleHealthChanged);
	HealthComponent->OnDeath.AddDynamic(this, &AASEnemyCharacter::HandleDeath);
}

bool AASEnemyCharacter::IsAlive() const
{
	return HealthComponent && !HealthComponent->IsDead();
}

void AASEnemyCharacter::TryAttack(AActor* Target)
{
	const float Now = GetWorld()->GetTimeSeconds();
	if (!Target || !IsAlive() || Now < LastAttackTime + AttackCooldown)
	{
		return;
	}

	LastAttackTime = Now;
	UGameplayStatics::ApplyDamage(Target, Tuning.ContactDamage, GetController(), this, UDamageType::StaticClass());
}

void AASEnemyCharacter::HandleHealthChanged(UASHealthComponent* ChangedComponent, float NewHealth, float Delta)
{
	if (Delta < 0.f && NewHealth > 0.f)
	{
		ASVisuals::SetColor(BodyMesh, FLinearColor::White);
		GetWorldTimerManager().SetTimer(HitFlashTimer, this, &AASEnemyCharacter::EndHitFlash, HitFlashTime, false);
	}
}

void AASEnemyCharacter::EndHitFlash()
{
	ASVisuals::SetColor(BodyMesh, Tuning.Color);
}

void AASEnemyCharacter::HandleDeath(UASHealthComponent* DeadComponent, AController* Killer)
{
	GetWorldTimerManager().ClearTimer(HitFlashTimer);
	GetCharacterMovement()->DisableMovement();
	SetActorEnableCollision(false);
	DetachFromControllerPendingDestroy();

	// Short delay so death listeners can still read the actor this frame.
	SetLifeSpan(0.1f);
}
