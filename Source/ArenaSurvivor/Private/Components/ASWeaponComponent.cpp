#include "Components/ASWeaponComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Weapons/ASProjectile.h"

UASWeaponComponent::UASWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ProjectileClass = AASProjectile::StaticClass();
}

void UASWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimer);
		World->GetTimerManager().ClearTimer(BoostTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void UASWeaponComponent::StartFire()
{
	bWantsToFire = true;
	RestartFireTimer();
}

void UASWeaponComponent::StopFire()
{
	bWantsToFire = false;
	GetWorld()->GetTimerManager().ClearTimer(FireTimer);
}

void UASWeaponComponent::RestartFireTimer()
{
	UWorld* World = GetWorld();
	if (!World || !bWantsToFire)
	{
		return;
	}

	// Respect the cooldown so tapping the trigger can't beat the fire rate.
	const float Interval = GetFireInterval();
	const float Delay = FMath::Max(0.f, LastFireTime + Interval - World->GetTimeSeconds());
	if (Delay <= 0.f)
	{
		Fire();
		World->GetTimerManager().SetTimer(FireTimer, this, &UASWeaponComponent::Fire, Interval, true);
	}
	else
	{
		World->GetTimerManager().SetTimer(FireTimer, this, &UASWeaponComponent::Fire, Interval, true, Delay);
	}
}

void UASWeaponComponent::Fire()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	UWorld* World = GetWorld();
	if (!OwnerPawn || !World || !ProjectileClass)
	{
		return;
	}

	LastFireTime = World->GetTimeSeconds();

	FRotator AimRotation = OwnerPawn->GetActorRotation();
	AimRotation.Pitch = 0.f;
	AimRotation.Roll = 0.f;
	AimRotation.Yaw += FMath::FRandRange(-SpreadDegrees, SpreadDegrees);

	const FVector SpawnLocation = OwnerPawn->GetActorTransform().TransformPosition(MuzzleOffset);

	FActorSpawnParameters Params;
	Params.Owner = OwnerPawn;
	Params.Instigator = OwnerPawn;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AASProjectile* Projectile = World->SpawnActor<AASProjectile>(ProjectileClass, SpawnLocation, AimRotation, Params))
	{
		Projectile->SetDamage(Damage);
	}
}

void UASWeaponComponent::ApplyFireRateBoost(float Multiplier, float Duration)
{
	FireRateMultiplier = FMath::Max(0.1f, Multiplier);
	GetWorld()->GetTimerManager().SetTimer(BoostTimer, this, &UASWeaponComponent::ClearBoost, Duration, false);
	RestartFireTimer();
}

void UASWeaponComponent::ClearBoost()
{
	FireRateMultiplier = 1.f;
	RestartFireTimer();
}

float UASWeaponComponent::GetBoostTimeRemaining() const
{
	const UWorld* World = GetWorld();
	return World ? FMath::Max(0.f, World->GetTimerManager().GetTimerRemaining(BoostTimer)) : 0.f;
}
