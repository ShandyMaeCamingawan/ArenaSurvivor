#include "Player/ASPlayerCharacter.h"
#include "ASVisuals.h"
#include "Camera/CameraComponent.h"
#include "Components/ASHealthComponent.h"
#include "Components/ASWeaponComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"

AASPlayerCharacter::AASPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// The controller owns the aim; the body just follows its yaw.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = false;
	Move->MaxWalkSpeed = 600.f;
	Move->MaxAcceleration = 4096.f;
	Move->BrakingDecelerationWalking = 4096.f;
	Move->GroundFriction = 10.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->TargetArmLength = 1700.f;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 6.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->SetFieldOfView(55.f);

	UMaterialInterface* ShapeMaterial = ASVisuals::FindShapeMaterial();

	// Basic shapes are 100 units across, so scale to fill the capsule.
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetStaticMesh(ASVisuals::FindShape(ASVisuals::CylinderPath));
	BodyMesh->SetMaterial(0, ShapeMaterial);
	BodyMesh->SetRelativeScale3D(FVector(0.84f, 0.84f, 1.92f));

	NoseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoseMesh"));
	NoseMesh->SetupAttachment(RootComponent);
	NoseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NoseMesh->SetStaticMesh(ASVisuals::FindShape(ASVisuals::CubePath));
	NoseMesh->SetMaterial(0, ShapeMaterial);
	NoseMesh->SetRelativeLocation(FVector(50.f, 0.f, 20.f));
	NoseMesh->SetRelativeScale3D(FVector(0.5f, 0.2f, 0.2f));

	HealthComponent = CreateDefaultSubobject<UASHealthComponent>(TEXT("HealthComponent"));
	WeaponComponent = CreateDefaultSubobject<UASWeaponComponent>(TEXT("WeaponComponent"));
}

void AASPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	ASVisuals::SetColor(BodyMesh, BodyColor);
	ASVisuals::SetColor(NoseMesh, FLinearColor::White);

	HealthComponent->OnDeath.AddDynamic(this, &AASPlayerCharacter::HandleDeath);
}

bool AASPlayerCharacter::IsAlive() const
{
	return HealthComponent && !HealthComponent->IsDead();
}

void AASPlayerCharacter::HandleDeath(UASHealthComponent* DeadComponent, AController* Killer)
{
	WeaponComponent->StopFire();
	GetCharacterMovement()->DisableMovement();
	ASVisuals::SetColor(BodyMesh, FLinearColor(0.15f, 0.15f, 0.15f));
}

bool AASPlayerCharacter::TryDash(FVector Direction)
{
	if (!IsAlive() || GetDashCooldownRemaining() > 0.f)
	{
		return false;
	}

	Direction.Z = 0.f;
	if (!Direction.Normalize())
	{
		Direction = GetActorForwardVector();
	}

	LastDashTime = GetWorld()->GetTimeSeconds();
	LaunchCharacter(Direction * DashSpeed + FVector(0.f, 0.f, DashLift), true, true);

	HealthComponent->SetInvulnerable(true);
	GetWorldTimerManager().SetTimer(DashInvulnerabilityTimer, this, &AASPlayerCharacter::EndDashInvulnerability, DashInvulnerabilityTime, false);
	return true;
}

void AASPlayerCharacter::EndDashInvulnerability()
{
	HealthComponent->SetInvulnerable(false);
}

float AASPlayerCharacter::GetDashCooldownRemaining() const
{
	return FMath::Max(0.f, LastDashTime + DashCooldown - GetWorld()->GetTimeSeconds());
}
