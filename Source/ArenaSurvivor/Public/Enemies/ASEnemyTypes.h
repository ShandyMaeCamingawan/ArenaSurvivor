#pragma once

#include "CoreMinimal.h"
#include "ASEnemyTypes.generated.h"

UENUM(BlueprintType)
enum class EASEnemyType : uint8
{
	/** Baseline chaser. */
	Grunt,
	/** Fragile but faster than the player; forces the use of dash. */
	Runner,
	/** Slow damage sponge that hits hard. */
	Brute
};

USTRUCT(BlueprintType)
struct ARENASURVIVOR_API FASEnemyTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MaxHealth = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MoveSpeed = 380.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float ContactDamage = 10.f;

	/** Uniform size multiplier applied to the capsule and body. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float Size = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	int32 ScoreValue = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PickupDropChance = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FLinearColor Color = FLinearColor::Red;

	static FASEnemyTuning ForType(EASEnemyType Type);
};
