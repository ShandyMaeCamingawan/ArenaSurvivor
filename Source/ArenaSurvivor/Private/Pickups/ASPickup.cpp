#include "Pickups/ASPickup.h"
#include "ASVisuals.h"
#include "Components/ASHealthComponent.h"
#include "Components/ASWeaponComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Player/ASPlayerCharacter.h"

AASPickup::AASPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 12.f;

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->InitSphereRadius(60.f);
	Trigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Trigger->SetGenerateOverlapEvents(true);
	Trigger->SetCanEverAffectNavigation(false);
	RootComponent = Trigger;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Trigger);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetStaticMesh(ASVisuals::FindShape(ASVisuals::CubePath));
	Mesh->SetMaterial(0, ASVisuals::FindShapeMaterial());
	Mesh->SetRelativeRotation(FRotator(45.f, 0.f, 45.f));
	Mesh->SetRelativeScale3D(FVector(0.4f));

	Rotator = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("Rotator"));
	Rotator->RotationRate = FRotator(0.f, 180.f, 0.f);
}

void AASPickup::BeginPlay()
{
	Super::BeginPlay();

	const FLinearColor Color = PickupType == EASPickupType::Health
		? FLinearColor(0.1f, 0.9f, 0.3f)
		: FLinearColor(1.f, 0.85f, 0.1f);
	ASVisuals::SetColor(Mesh, Color);

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AASPickup::HandleOverlap);
}

void AASPickup::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AASPlayerCharacter* PlayerCharacter = Cast<AASPlayerCharacter>(OtherActor);
	if (PlayerCharacter && PlayerCharacter->IsAlive() && ApplyTo(PlayerCharacter))
	{
		Destroy();
	}
}

bool AASPickup::ApplyTo(AASPlayerCharacter* PlayerCharacter)
{
	switch (PickupType)
	{
	case EASPickupType::Health:
		return PlayerCharacter->GetHealthComponent()->Heal(HealAmount) > 0.f;

	case EASPickupType::RapidFire:
		PlayerCharacter->GetWeaponComponent()->ApplyFireRateBoost(RapidFireMultiplier, RapidFireDuration);
		return true;
	}

	return false;
}
