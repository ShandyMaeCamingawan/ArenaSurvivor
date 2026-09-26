#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ASHUD.generated.h"

class AASGameState;
class AASPlayerCharacter;
class UFont;

/** Canvas-drawn HUD, so no UMG assets are needed. Layout is authored at 1080p and scaled. */
UCLASS()
class ARENASURVIVOR_API AASHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawPlayerStatus(const AASPlayerCharacter* PlayerCharacter);
	void DrawMatchStatus(const AASGameState* State);
	void DrawCenterMessages(const AASGameState* State);

	void DrawBar(float X, float Y, float Width, float Height, float Fraction, const FLinearColor& FillColor);
	void DrawTextScaled(const FString& Text, float X, float Y, UFont* Font, float Scale, const FLinearColor& Color);
	void DrawCenteredText(const FString& Text, float Y, UFont* Font, float Scale, const FLinearColor& Color);

	float UIScale = 1.f;
};
