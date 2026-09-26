#include "Components/ASHealthComponent.h"
#include "GameFramework/Actor.h"

UASHealthComponent::UASHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UASHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UASHealthComponent::HandleTakeAnyDamage);
	}
}

void UASHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.f || bIsDead || bInvulnerable)
	{
		return;
	}

	ApplyHealthDelta(-Damage, InstigatedBy);
}

float UASHealthComponent::Heal(float Amount)
{
	if (Amount <= 0.f || bIsDead)
	{
		return 0.f;
	}

	const float OldHealth = Health;
	ApplyHealthDelta(Amount, nullptr);
	return Health - OldHealth;
}

void UASHealthComponent::SetMaxHealth(float NewMaxHealth, bool bRefill)
{
	MaxHealth = FMath::Max(1.f, NewMaxHealth);
	Health = bRefill ? MaxHealth : FMath::Min(Health, MaxHealth);
}

void UASHealthComponent::ApplyHealthDelta(float Delta, AController* Instigator)
{
	const float OldHealth = Health;
	Health = FMath::Clamp(Health + Delta, 0.f, MaxHealth);

	const float ActualDelta = Health - OldHealth;
	if (!FMath::IsNearlyZero(ActualDelta))
	{
		OnHealthChanged.Broadcast(this, Health, ActualDelta);
	}

	if (Health <= 0.f && !bIsDead)
	{
		bIsDead = true;
		OnDeath.Broadcast(this, Instigator);
	}
}
