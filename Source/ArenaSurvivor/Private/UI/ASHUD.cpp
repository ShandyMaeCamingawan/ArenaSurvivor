#include "UI/ASHUD.h"
#include "Components/ASHealthComponent.h"
#include "Components/ASWeaponComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "Game/ASGameState.h"
#include "Player/ASPlayerCharacter.h"

namespace
{
	const FLinearColor TextColor(0.95f, 0.95f, 0.95f);
	const FLinearColor AccentColor(1.f, 0.8f, 0.2f);
	const FLinearColor PanelColor(0.f, 0.f, 0.f, 0.45f);
	constexpr float Margin = 32.f;
}

void AASHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	UIScale = Canvas->ClipY / 1080.f;

	const AASGameState* State = GetWorld()->GetGameState<AASGameState>();

	if (const AASPlayerCharacter* PlayerCharacter = Cast<AASPlayerCharacter>(GetOwningPawn()))
	{
		DrawPlayerStatus(PlayerCharacter);
	}

	if (State)
	{
		DrawMatchStatus(State);
		DrawCenterMessages(State);
	}
}

void AASHUD::DrawPlayerStatus(const AASPlayerCharacter* PlayerCharacter)
{
	const UASHealthComponent* Health = PlayerCharacter->GetHealthComponent();
	const float BarWidth = 360.f * UIScale;
	const float BarHeight = 22.f * UIScale;
	const float X = Margin * UIScale;
	float Y = Canvas->ClipY - Margin * UIScale - BarHeight;

	const float HealthFraction = Health->GetHealthFraction();
	const FLinearColor HealthColor = FLinearColor::LerpUsingHSV(FLinearColor(0.9f, 0.1f, 0.1f), FLinearColor(0.1f, 0.85f, 0.3f), HealthFraction);
	DrawBar(X, Y, BarWidth, BarHeight, HealthFraction, HealthColor);
	DrawTextScaled(FString::Printf(TEXT("%.0f / %.0f"), Health->GetHealth(), Health->GetMaxHealth()), X + 8.f * UIScale, Y + 1.f * UIScale, GEngine->GetSmallFont(), 1.1f, TextColor);

	// Dash readiness sits above the health bar.
	Y -= BarHeight * 0.5f + 10.f * UIScale;
	const float DashCooldown = PlayerCharacter->GetDashCooldown();
	const float DashFraction = DashCooldown > 0.f ? 1.f - PlayerCharacter->GetDashCooldownRemaining() / DashCooldown : 1.f;
	DrawBar(X, Y, BarWidth * 0.5f, BarHeight * 0.5f, DashFraction, DashFraction >= 1.f ? FLinearColor(0.3f, 0.7f, 1.f) : FLinearColor(0.25f, 0.3f, 0.4f));
	DrawTextScaled(TEXT("DASH"), X + BarWidth * 0.5f + 10.f * UIScale, Y - 3.f * UIScale, GEngine->GetSmallFont(), 1.f, TextColor);

	const float BoostRemaining = PlayerCharacter->GetWeaponComponent()->GetBoostTimeRemaining();
	if (BoostRemaining > 0.f)
	{
		DrawTextScaled(FString::Printf(TEXT("RAPID FIRE  %.1fs"), BoostRemaining), X, Y - 34.f * UIScale, GEngine->GetMediumFont(), 1.f, AccentColor);
	}
}

void AASHUD::DrawMatchStatus(const AASGameState* State)
{
	const float X = Margin * UIScale;
	const float Y = Margin * UIScale;
	UFont* Font = GEngine->GetMediumFont();

	DrawTextScaled(FString::Printf(TEXT("WAVE %d"), FMath::Max(1, State->Wave)), X, Y, GEngine->GetLargeFont(), 1.2f, AccentColor);
	DrawTextScaled(FString::Printf(TEXT("Score  %d"), State->Score), X, Y + 44.f * UIScale, Font, 1.f, TextColor);
	DrawTextScaled(FString::Printf(TEXT("Enemies  %d"), State->EnemiesRemaining), X, Y + 72.f * UIScale, Font, 1.f, TextColor);
}

void AASHUD::DrawCenterMessages(const AASGameState* State)
{
	const float CenterY = Canvas->ClipY * 0.3f;

	if (State->bGameOver)
	{
		DrawRect(PanelColor, 0.f, CenterY - 20.f * UIScale, Canvas->ClipX, 190.f * UIScale);
		DrawCenteredText(TEXT("YOU DIED"), CenterY, GEngine->GetLargeFont(), 2.5f, FLinearColor(0.95f, 0.2f, 0.2f));
		DrawCenteredText(FString::Printf(TEXT("Wave %d   Score %d   Kills %d"), State->Wave, State->Score, State->Kills), CenterY + 80.f * UIScale, GEngine->GetMediumFont(), 1.2f, TextColor);
		DrawCenteredText(TEXT("Press R to try again"), CenterY + 120.f * UIScale, GEngine->GetMediumFont(), 1.f, AccentColor);
		return;
	}

	if (State->bIntermission)
	{
		const float SecondsLeft = FMath::Max(0.f, State->NextWaveTime - GetWorld()->GetTimeSeconds());
		DrawCenteredText(FString::Printf(TEXT("WAVE %d"), State->Wave + 1), CenterY, GEngine->GetLargeFont(), 2.f, AccentColor);
		DrawCenteredText(FString::Printf(TEXT("starts in %d"), FMath::CeilToInt(SecondsLeft)), CenterY + 60.f * UIScale, GEngine->GetMediumFont(), 1.2f, TextColor);
	}
}

void AASHUD::DrawBar(float X, float Y, float Width, float Height, float Fraction, const FLinearColor& FillColor)
{
	DrawRect(PanelColor, X, Y, Width, Height);
	DrawRect(FillColor, X, Y, Width * FMath::Clamp(Fraction, 0.f, 1.f), Height);
}

void AASHUD::DrawTextScaled(const FString& Text, float X, float Y, UFont* Font, float Scale, const FLinearColor& Color)
{
	DrawText(Text, Color, X, Y, Font, Scale * UIScale);
}

void AASHUD::DrawCenteredText(const FString& Text, float Y, UFont* Font, float Scale, const FLinearColor& Color)
{
	float Width = 0.f;
	float Height = 0.f;
	GetTextSize(Text, Width, Height, Font, Scale * UIScale);
	DrawText(Text, Color, (Canvas->ClipX - Width) * 0.5f, Y, Font, Scale * UIScale);
}
