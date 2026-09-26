#include "Enemies/ASEnemyTypes.h"

FASEnemyTuning FASEnemyTuning::ForType(EASEnemyType Type)
{
	FASEnemyTuning Tuning;

	switch (Type)
	{
	case EASEnemyType::Runner:
		Tuning.MaxHealth = 25.f;
		Tuning.MoveSpeed = 640.f;
		Tuning.ContactDamage = 6.f;
		Tuning.Size = 0.75f;
		Tuning.ScoreValue = 15;
		Tuning.PickupDropChance = 0.08f;
		Tuning.Color = FLinearColor(1.f, 0.5f, 0.f);
		break;

	case EASEnemyType::Brute:
		Tuning.MaxHealth = 240.f;
		Tuning.MoveSpeed = 240.f;
		Tuning.ContactDamage = 25.f;
		Tuning.Size = 1.6f;
		Tuning.ScoreValue = 50;
		Tuning.PickupDropChance = 0.5f;
		Tuning.Color = FLinearColor(0.45f, 0.1f, 0.75f);
		break;

	case EASEnemyType::Grunt:
	default:
		Tuning.MaxHealth = 50.f;
		Tuning.MoveSpeed = 380.f;
		Tuning.ContactDamage = 10.f;
		Tuning.Size = 1.f;
		Tuning.ScoreValue = 10;
		Tuning.PickupDropChance = 0.1f;
		Tuning.Color = FLinearColor(0.85f, 0.1f, 0.1f);
		break;
	}

	return Tuning;
}
