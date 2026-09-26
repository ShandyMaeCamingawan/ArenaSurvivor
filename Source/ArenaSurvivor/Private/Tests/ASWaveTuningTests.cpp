#include "Enemies/ASEnemyTypes.h"
#include "Game/ASGameMode.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// A macro rather than a typed constant: EAutomationTestFlags changed from a namespaced
// enum to an enum class in 5.5, and the plain expression compiles against both.
#define AS_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FASWaveEnemyCountTest, "ArenaSurvivor.Waves.EnemyCountGrows", AS_TEST_FLAGS)

bool FASWaveEnemyCountTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Wave 1 spawns enemies"), AASGameMode::GetEnemyCountForWave(1) > 0);
	TestEqual(TEXT("Wave 0 is clamped to wave 1"), AASGameMode::GetEnemyCountForWave(0), AASGameMode::GetEnemyCountForWave(1));

	for (int32 Wave = 1; Wave < 50; ++Wave)
	{
		TestTrue(FString::Printf(TEXT("Wave %d has more enemies than wave %d"), Wave + 1, Wave),
			AASGameMode::GetEnemyCountForWave(Wave + 1) > AASGameMode::GetEnemyCountForWave(Wave));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FASWaveHealthScalingTest, "ArenaSurvivor.Waves.HealthScaling", AS_TEST_FLAGS)

bool FASWaveHealthScalingTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Wave 1 enemies use base health"), AASGameMode::GetEnemyHealthMultiplierForWave(1), 1.f);

	for (int32 Wave = 1; Wave < 50; ++Wave)
	{
		TestTrue(FString::Printf(TEXT("Wave %d enemies are tougher than wave %d"), Wave + 1, Wave),
			AASGameMode::GetEnemyHealthMultiplierForWave(Wave + 1) > AASGameMode::GetEnemyHealthMultiplierForWave(Wave));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FASWaveEnemyMixTest, "ArenaSurvivor.Waves.EnemyMix", AS_TEST_FLAGS)

bool FASWaveEnemyMixTest::RunTest(const FString& Parameters)
{
	constexpr int32 Samples = 100;

	for (int32 Sample = 0; Sample < Samples; ++Sample)
	{
		const float Roll = static_cast<float>(Sample) / Samples;
		TestEqual(TEXT("Wave 1 is grunts only"), AASGameMode::PickEnemyTypeForWave(1, Roll), EASEnemyType::Grunt);
		TestNotEqual(TEXT("No brutes before wave 4"), AASGameMode::PickEnemyTypeForWave(3, Roll), EASEnemyType::Brute);
	}

	// Late waves still have grunts as the most common type.
	int32 Grunts = 0;
	for (int32 Sample = 0; Sample < Samples; ++Sample)
	{
		Grunts += AASGameMode::PickEnemyTypeForWave(100, static_cast<float>(Sample) / Samples) == EASEnemyType::Grunt ? 1 : 0;
	}
	TestTrue(TEXT("Grunts remain the plurality in late waves"), Grunts >= Samples * 0.4f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FASEnemyTuningTest, "ArenaSurvivor.Enemies.TuningRoles", AS_TEST_FLAGS)

bool FASEnemyTuningTest::RunTest(const FString& Parameters)
{
	const FASEnemyTuning Grunt = FASEnemyTuning::ForType(EASEnemyType::Grunt);
	const FASEnemyTuning Runner = FASEnemyTuning::ForType(EASEnemyType::Runner);
	const FASEnemyTuning Brute = FASEnemyTuning::ForType(EASEnemyType::Brute);

	TestTrue(TEXT("Runners are faster than grunts"), Runner.MoveSpeed > Grunt.MoveSpeed);
	TestTrue(TEXT("Runners are frailer than grunts"), Runner.MaxHealth < Grunt.MaxHealth);
	TestTrue(TEXT("Brutes are tougher than grunts"), Brute.MaxHealth > Grunt.MaxHealth);
	TestTrue(TEXT("Brutes hit harder than grunts"), Brute.ContactDamage > Grunt.ContactDamage);
	TestTrue(TEXT("Brutes are worth more than grunts"), Brute.ScoreValue > Grunt.ScoreValue);

	return true;
}

#undef AS_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
