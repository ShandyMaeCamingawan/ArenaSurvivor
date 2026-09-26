#include "Game/ASGameMode.h"
#include "ArenaSurvivor.h"
#include "ASVisuals.h"
#include "Components/ASHealthComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Enemies/ASEnemyCharacter.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Game/ASGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Pickups/ASPickup.h"
#include "Player/ASPlayerCharacter.h"
#include "Player/ASPlayerController.h"
#include "TimerManager.h"
#include "UI/ASHUD.h"
#include "UObject/ConstructorHelpers.h"

AASGameMode::AASGameMode()
{
	DefaultPawnClass = AASPlayerCharacter::StaticClass();
	PlayerControllerClass = AASPlayerController::StaticClass();
	GameStateClass = AASGameState::StaticClass();
	HUDClass = AASHUD::StaticClass();

	EnemyClass = AASEnemyCharacter::StaticClass();
	PickupClass = AASPickup::StaticClass();

	CubeMesh = ASVisuals::FindShape(ASVisuals::CubePath);
	ShapeMaterial = ASVisuals::FindShapeMaterial();
}

// ---------------------------------------------------------------------------
// Wave tuning

int32 AASGameMode::GetEnemyCountForWave(int32 Wave)
{
	Wave = FMath::Max(1, Wave);
	return 5 + 3 * Wave + (Wave * Wave) / 6;
}

float AASGameMode::GetEnemyHealthMultiplierForWave(int32 Wave)
{
	return 1.f + 0.08f * FMath::Max(0, Wave - 1);
}

EASEnemyType AASGameMode::PickEnemyTypeForWave(int32 Wave, float Roll)
{
	// Runners from wave 2, brutes from wave 4; both capped so grunts stay the majority.
	const float BruteChance = FMath::Clamp((Wave - 3) * 0.04f, 0.f, 0.25f);
	const float RunnerChance = FMath::Clamp((Wave - 1) * 0.06f, 0.f, 0.35f);

	if (Roll < BruteChance)
	{
		return EASEnemyType::Brute;
	}
	if (Roll < BruteChance + RunnerChance)
	{
		return EASEnemyType::Runner;
	}
	return EASEnemyType::Grunt;
}

// ---------------------------------------------------------------------------
// Match flow

void AASGameMode::StartPlay()
{
	// Build before BeginPlay runs anywhere so the already-spawned player lands on the floor.
	BuildArena();

	Super::StartPlay();

	ScheduleNextWave(FirstWaveDelay);
}

void AASGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	if (AASPlayerCharacter* PlayerCharacter = Cast<AASPlayerCharacter>(PlayerPawn))
	{
		PlayerCharacter->GetHealthComponent()->OnDeath.AddUniqueDynamic(this, &AASGameMode::HandlePlayerDeath);
	}
}

AASGameState* AASGameMode::GetArenaState() const
{
	return GetGameState<AASGameState>();
}

void AASGameMode::ScheduleNextWave(float Delay)
{
	if (AASGameState* State = GetArenaState())
	{
		State->bIntermission = true;
		State->NextWaveTime = GetWorld()->GetTimeSeconds() + Delay;
	}

	GetWorldTimerManager().SetTimer(WaveTimer, this, &AASGameMode::StartNextWave, Delay, false);
}

void AASGameMode::StartNextWave()
{
	AASGameState* State = GetArenaState();
	if (!State || State->bGameOver)
	{
		return;
	}

	++State->Wave;
	State->bIntermission = false;
	PendingSpawns = GetEnemyCountForWave(State->Wave);
	RefreshEnemiesRemaining();

	UE_LOG(LogArenaSurvivor, Log, TEXT("Wave %d: %d enemies"), State->Wave, PendingSpawns);

	GetWorldTimerManager().SetTimer(SpawnTimer, this, &AASGameMode::SpawnTick, SpawnInterval, true, 0.f);
}

void AASGameMode::SpawnTick()
{
	const AASGameState* State = GetArenaState();
	if (!State || State->bGameOver || PendingSpawns <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimer);
		return;
	}

	if (AliveEnemies >= MaxAliveEnemies)
	{
		return;
	}

	FVector Location;
	if (!FindSpawnLocation(Location))
	{
		return;
	}

	--PendingSpawns;
	SpawnEnemy(Location, PickEnemyTypeForWave(State->Wave, FMath::FRand()));
}

bool AASGameMode::FindSpawnLocation(FVector& OutLocation) const
{
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	const float Margin = WallThickness + 150.f;
	const float Extent = ArenaHalfExtent - Margin;

	for (int32 Attempt = 0; Attempt < 12; ++Attempt)
	{
		const FVector Candidate(FMath::FRandRange(-Extent, Extent), FMath::FRandRange(-Extent, Extent), FloorTopZ + 150.f);
		if (!PlayerPawn || FVector::Dist2D(Candidate, PlayerPawn->GetActorLocation()) >= MinSpawnDistanceFromPlayer)
		{
			OutLocation = Candidate;
			return true;
		}
	}

	return false;
}

void AASGameMode::SpawnEnemy(const FVector& Location, EASEnemyType Type)
{
	const AASGameState* State = GetArenaState();
	const FTransform SpawnTransform(FRotator::ZeroRotator, Location);

	AASEnemyCharacter* Enemy = GetWorld()->SpawnActorDeferred<AASEnemyCharacter>(
		EnemyClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!Enemy)
	{
		return;
	}

	Enemy->InitFromType(Type, GetEnemyHealthMultiplierForWave(State ? State->Wave : 1));
	Enemy->GetHealthComponent()->OnDeath.AddDynamic(this, &AASGameMode::HandleEnemyDeath);
	Enemy->FinishSpawning(SpawnTransform);

	++AliveEnemies;
	RefreshEnemiesRemaining();
}

void AASGameMode::HandleEnemyDeath(UASHealthComponent* HealthComponent, AController* Killer)
{
	AASGameState* State = GetArenaState();
	const AASEnemyCharacter* Enemy = HealthComponent ? Cast<AASEnemyCharacter>(HealthComponent->GetOwner()) : nullptr;
	if (!State || !Enemy)
	{
		return;
	}

	AliveEnemies = FMath::Max(0, AliveEnemies - 1);
	RefreshEnemiesRemaining();

	if (State->bGameOver)
	{
		return;
	}

	State->Score += Enemy->GetTuning().ScoreValue * State->Wave;
	++State->Kills;

	TrySpawnPickup(Enemy->GetActorLocation(), Enemy->GetTuning().PickupDropChance);

	if (AliveEnemies == 0 && PendingSpawns == 0)
	{
		ScheduleNextWave(IntermissionTime);
	}
}

void AASGameMode::HandlePlayerDeath(UASHealthComponent* HealthComponent, AController* Killer)
{
	AASGameState* State = GetArenaState();
	if (!State || State->bGameOver)
	{
		return;
	}

	State->bGameOver = true;
	State->bIntermission = false;
	GetWorldTimerManager().ClearTimer(WaveTimer);
	GetWorldTimerManager().ClearTimer(SpawnTimer);

	UE_LOG(LogArenaSurvivor, Log, TEXT("Game over on wave %d with score %d"), State->Wave, State->Score);
}

void AASGameMode::RefreshEnemiesRemaining()
{
	if (AASGameState* State = GetArenaState())
	{
		State->EnemiesRemaining = PendingSpawns + AliveEnemies;
	}
}

void AASGameMode::TrySpawnPickup(const FVector& Location, float Chance)
{
	if (!PickupClass || FMath::FRand() >= Chance)
	{
		return;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, FVector(Location.X, Location.Y, FloorTopZ + 50.f));
	AASPickup* Pickup = GetWorld()->SpawnActorDeferred<AASPickup>(
		PickupClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Pickup)
	{
		Pickup->SetPickupType(FMath::FRand() < RapidFireDropShare ? EASPickupType::RapidFire : EASPickupType::Health);
		Pickup->FinishSpawning(SpawnTransform);
	}
}

// ---------------------------------------------------------------------------
// Arena construction

void AASGameMode::BuildArena()
{
	const float Side = ArenaHalfExtent * 2.f;
	const float WallCenterZ = FloorTopZ + WallHeight * 0.5f;
	const float WallOffset = ArenaHalfExtent + WallThickness * 0.5f;
	const FLinearColor FloorColor(0.18f, 0.19f, 0.22f);
	const FLinearColor WallColor(0.35f, 0.37f, 0.42f);

	SpawnBlock(FVector(0.f, 0.f, FloorTopZ - 50.f), FVector(Side + WallThickness * 2.f, Side + WallThickness * 2.f, 100.f), FloorColor);

	SpawnBlock(FVector(WallOffset, 0.f, WallCenterZ), FVector(WallThickness, Side, WallHeight), WallColor);
	SpawnBlock(FVector(-WallOffset, 0.f, WallCenterZ), FVector(WallThickness, Side, WallHeight), WallColor);
	SpawnBlock(FVector(0.f, WallOffset, WallCenterZ), FVector(Side + WallThickness * 2.f, WallThickness, WallHeight), WallColor);
	SpawnBlock(FVector(0.f, -WallOffset, WallCenterZ), FVector(Side + WallThickness * 2.f, WallThickness, WallHeight), WallColor);

	// A few pillars to break line of movement; kept clear of the centre spawn.
	const float PillarOffset = ArenaHalfExtent * 0.5f;
	const FVector PillarSize(160.f, 160.f, WallHeight);
	for (const FVector2D& Corner : { FVector2D(1, 1), FVector2D(1, -1), FVector2D(-1, 1), FVector2D(-1, -1) })
	{
		SpawnBlock(FVector(Corner.X * PillarOffset, Corner.Y * PillarOffset, WallCenterZ), PillarSize, WallColor);
	}

	SpawnLights();
}

AStaticMeshActor* AASGameMode::SpawnBlock(const FVector& Center, const FVector& Size, const FLinearColor& Color)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Center, FRotator::ZeroRotator, Params);
	if (!Block)
	{
		return nullptr;
	}

	Block->SetMobility(EComponentMobility::Movable);

	UStaticMeshComponent* MeshComponent = Block->GetStaticMeshComponent();
	MeshComponent->SetStaticMesh(CubeMesh);
	MeshComponent->SetMaterial(0, ShapeMaterial);
	// The engine cube is 100 units on each side.
	Block->SetActorScale3D(Size / 100.f);
	ASVisuals::SetColor(MeshComponent, Color);

	return Block;
}

void AASGameMode::SpawnLights()
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-55.f, 35.f, 0.f), Params))
	{
		UDirectionalLightComponent* Light = CastChecked<UDirectionalLightComponent>(Sun->GetLightComponent());
		Light->SetMobility(EComponentMobility::Movable);
		Light->SetIntensity(6.f);
		Light->SetLightColor(FLinearColor(1.f, 0.96f, 0.9f));
	}

	// Shadowless fill from the opposite side so the unlit faces aren't black.
	if (ADirectionalLight* Fill = GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-35.f, 215.f, 0.f), Params))
	{
		UDirectionalLightComponent* Light = CastChecked<UDirectionalLightComponent>(Fill->GetLightComponent());
		Light->SetMobility(EComponentMobility::Movable);
		Light->SetIntensity(1.5f);
		Light->SetCastShadows(false);
		Light->SetLightColor(FLinearColor(0.6f, 0.7f, 1.f));
	}
}
