#include "Player/ASPlayerController.h"
#include "ArenaSurvivor.h"
#include "Components/ASWeaponComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Player/ASPlayerCharacter.h"

namespace
{
	UInputAction* MakeAction(UObject* Outer, const TCHAR* Name, EInputActionValueType ValueType)
	{
		UInputAction* Action = NewObject<UInputAction>(Outer, Name);
		Action->ValueType = ValueType;
		return Action;
	}

	template <typename TModifier>
	TModifier* AddModifier(UObject* Outer, FEnhancedActionKeyMapping& Mapping)
	{
		TModifier* Modifier = NewObject<TModifier>(Outer);
		Mapping.Modifiers.Add(Modifier);
		return Modifier;
	}

	/** Maps a digital key onto one direction of a 2D axis action. */
	void MapDirectionalKey(UObject* Outer, UInputMappingContext* Context, UInputAction* Action, const FKey& Key, bool bVertical, bool bNegative)
	{
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);
		if (bVertical)
		{
			AddModifier<UInputModifierSwizzleAxis>(Outer, Mapping)->Order = EInputAxisSwizzle::YXZ;
		}
		if (bNegative)
		{
			AddModifier<UInputModifierNegate>(Outer, Mapping);
		}
	}
}

AASPlayerController::AASPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AASPlayerController::BuildInputAssets()
{
	if (DefaultMapping)
	{
		return;
	}

	MoveAction = MakeAction(this, TEXT("IA_Move"), EInputActionValueType::Axis2D);
	AimAction = MakeAction(this, TEXT("IA_Aim"), EInputActionValueType::Axis2D);
	FireAction = MakeAction(this, TEXT("IA_Fire"), EInputActionValueType::Boolean);
	RestartAction = MakeAction(this, TEXT("IA_Restart"), EInputActionValueType::Boolean);

	DefaultMapping = NewObject<UInputMappingContext>(this, TEXT("IMC_Default"));

	// Axis2D convention: X = right, Y = forward.
	MapDirectionalKey(this, DefaultMapping, MoveAction, EKeys::W, true, false);
	MapDirectionalKey(this, DefaultMapping, MoveAction, EKeys::S, true, true);
	MapDirectionalKey(this, DefaultMapping, MoveAction, EKeys::D, false, false);
	MapDirectionalKey(this, DefaultMapping, MoveAction, EKeys::A, false, true);
	MapDirectionalKey(this, DefaultMapping, MoveAction, EKeys::Up, true, false);
	MapDirectionalKey(this, DefaultMapping, MoveAction, EKeys::Down, true, true);
	MapDirectionalKey(this, DefaultMapping, MoveAction, EKeys::Right, false, false);
	MapDirectionalKey(this, DefaultMapping, MoveAction, EKeys::Left, false, true);
	AddModifier<UInputModifierDeadZone>(this, DefaultMapping->MapKey(MoveAction, EKeys::Gamepad_Left2D));

	AddModifier<UInputModifierDeadZone>(this, DefaultMapping->MapKey(AimAction, EKeys::Gamepad_Right2D));

	DefaultMapping->MapKey(FireAction, EKeys::LeftMouseButton);
	DefaultMapping->MapKey(FireAction, EKeys::Gamepad_RightTrigger);

	DefaultMapping->MapKey(RestartAction, EKeys::R);
	DefaultMapping->MapKey(RestartAction, EKeys::Gamepad_Special_Right);
}

void AASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	BuildInputAssets();

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (!Input)
	{
		UE_LOG(LogArenaSurvivor, Error, TEXT("Expected an EnhancedInputComponent. Check DefaultInputComponentClass in DefaultInput.ini."));
		return;
	}

	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AASPlayerController::HandleMove);
	Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &AASPlayerController::HandleMoveReleased);
	Input->BindAction(AimAction, ETriggerEvent::Triggered, this, &AASPlayerController::HandleAim);
	Input->BindAction(AimAction, ETriggerEvent::Completed, this, &AASPlayerController::HandleAimReleased);
	Input->BindAction(FireAction, ETriggerEvent::Started, this, &AASPlayerController::HandleFirePressed);
	Input->BindAction(FireAction, ETriggerEvent::Completed, this, &AASPlayerController::HandleFireReleased);
	Input->BindAction(RestartAction, ETriggerEvent::Started, this, &AASPlayerController::HandleRestart);
}

void AASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMapping, 0);
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(InputMode);
}

AASPlayerCharacter* AASPlayerController::GetLivingCharacter() const
{
	AASPlayerCharacter* PlayerCharacter = Cast<AASPlayerCharacter>(GetPawn());
	return PlayerCharacter && PlayerCharacter->IsAlive() ? PlayerCharacter : nullptr;
}

void AASPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateAim();
}

void AASPlayerController::UpdateAim()
{
	const AASPlayerCharacter* PlayerCharacter = GetLivingCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	const FVector PawnLocation = PlayerCharacter->GetActorLocation();
	FVector AimDirection = FVector::ZeroVector;

	if (!StickAim.IsNearlyZero())
	{
		AimDirection = FVector(StickAim.Y, StickAim.X, 0.f);
	}
	else
	{
		// Intersect the cursor ray with the horizontal plane at pawn height.
		FVector RayOrigin;
		FVector RayDirection;
		if (DeprojectMousePositionToWorld(RayOrigin, RayDirection) && !FMath::IsNearlyZero(RayDirection.Z))
		{
			const float Distance = (PawnLocation.Z - RayOrigin.Z) / RayDirection.Z;
			if (Distance > 0.f)
			{
				AimDirection = RayOrigin + RayDirection * Distance - PawnLocation;
			}
		}
	}

	AimDirection.Z = 0.f;
	if (AimDirection.SizeSquared() > FMath::Square(10.f))
	{
		SetControlRotation(AimDirection.Rotation());
	}
}

void AASPlayerController::HandleMove(const FInputActionValue& Value)
{
	MoveInput = Value.Get<FVector2D>();

	if (AASPlayerCharacter* PlayerCharacter = GetLivingCharacter())
	{
		// The camera never rotates, so movement is in world space.
		PlayerCharacter->AddMovementInput(FVector::ForwardVector, MoveInput.Y);
		PlayerCharacter->AddMovementInput(FVector::RightVector, MoveInput.X);
	}
}

void AASPlayerController::HandleMoveReleased(const FInputActionValue& Value)
{
	MoveInput = FVector2D::ZeroVector;
}

void AASPlayerController::HandleAim(const FInputActionValue& Value)
{
	StickAim = Value.Get<FVector2D>();
}

void AASPlayerController::HandleAimReleased(const FInputActionValue& Value)
{
	StickAim = FVector2D::ZeroVector;
}

void AASPlayerController::HandleFirePressed()
{
	if (AASPlayerCharacter* PlayerCharacter = GetLivingCharacter())
	{
		PlayerCharacter->GetWeaponComponent()->StartFire();
	}
}

void AASPlayerController::HandleFireReleased()
{
	if (AASPlayerCharacter* PlayerCharacter = Cast<AASPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->GetWeaponComponent()->StopFire();
	}
}

void AASPlayerController::HandleRestart()
{
	const AASPlayerCharacter* PlayerCharacter = Cast<AASPlayerCharacter>(GetPawn());
	if (!PlayerCharacter || !PlayerCharacter->IsAlive())
	{
		RestartLevel();
	}
}
