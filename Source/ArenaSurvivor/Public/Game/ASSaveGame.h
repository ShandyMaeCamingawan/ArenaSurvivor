#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ASSaveGame.generated.h"

UCLASS()
class ARENASURVIVOR_API UASSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static const FString SlotName;
	static constexpr int32 UserIndex = 0;

	UPROPERTY()
	int32 HighScore = 0;

	UPROPERTY()
	int32 BestWave = 0;
};
