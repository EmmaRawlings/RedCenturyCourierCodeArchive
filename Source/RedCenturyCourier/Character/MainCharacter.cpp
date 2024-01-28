// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"

#include "MainCharacterMovement.h"
#include "Engine/DamageEvents.h"
#include "../Utility/Utils.h"
#include "GameFramework/CharacterMovementComponent.h"

AMainCharacter::AMainCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMainCharacterMovement>(TEXT("CharMoveComp")))
{
	// Defaults
	bCanJumpWhilstCrouched = true;
	Health = 100.f;
	MaxHealth = 100.f;
	HealthRecovery = 10.f;
	OnTakeAnyDamage.AddDynamic(this, &AMainCharacter::TakeAnyDamage);
	CoyoteTime = 0.f;
}

// bool AMainCharacter::Collect(TScriptInterface<ICollectableInterface> Collectable)
// {
// 	if (!OnCollection.IsBound()) return false;
// 	return OnCollection.Execute(Collectable);
// }

APlayerController* AMainCharacter::GetPlayerController() const
{
	return Cast<APlayerController>(GetController());
}

bool AMainCharacter::IsInputForwards(const float maxAngle) const
{
	const FVector input = GetLastMovementInputVector();
	return input.Length() > UE_KINDA_SMALL_NUMBER && FUtils::AngleInDegrees(input, GetActorRotation().Vector()) <= maxAngle;
}

void AMainCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	
	switch(GetCharacterMovement()->MovementMode)
	{
	case MOVE_Walking:
		CoyoteTimeRemaining = CoyoteTime;
		break;
	default:
		break;
	}
}

void AMainCharacter::CheckJumpInput(float DeltaTime)
{
	JumpCurrentCountPreJump = JumpCurrentCount;

	if (GetCharacterMovement())
	{
		if (bPressedJump)
		{
			// If this is the first jump and we're already falling,
			// then increment the JumpCount to compensate.
			const bool bFirstJump = JumpCurrentCount == 0;
			if (bFirstJump
				&& GetCharacterMovement()->IsFalling()
				&& !Cast<UModularCharacterMovementComponent>(GetCharacterMovement())->NavModeAllowsFirstAirJump()
				&& CoyoteTimeRemaining <= 0.f)
			{
				JumpCurrentCount++;
			}

			const bool bDidJump = CanJump() && GetCharacterMovement()->DoJump(bClientUpdating);
			if (bDidJump)
			{
				// Transition from not (actively) jumping to jumping.
				if (!bWasJumping)
				{
					JumpCurrentCount++;
					JumpForceTimeRemaining = GetJumpMaxHoldTime();
					OnJumped();
				}
			}

			bWasJumping = bDidJump;
		}
	}
}

float AMainCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (DamageResistances.Contains(DamageEvent.DamageTypeClass))
	{
		DamageAmount *= 1 - DamageResistances[DamageEvent.DamageTypeClass];
		if (DamageAmount < 0.f) DamageAmount = 0.f;
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

float AMainCharacter::GetActorTimeDilation() const
{
	return Super::GetActorTimeDilation()
		* Cast<UMainCharacterMovement>(GetCharacterMovement())->GetRushTimeDilation();
}

float AMainCharacter::GetActorTimeDilation(const UWorld& ActorWorld) const
{
	return Super::GetActorTimeDilation(ActorWorld)
		* Cast<UMainCharacterMovement>(GetCharacterMovement())->GetRushTimeDilation();
}

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CoyoteTimeRemaining = CoyoteTime;
}

void AMainCharacter::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (HealthRecovery > 0.f && Health < MaxHealth)
	{
		Health += HealthRecovery * DeltaTime;
		if (Health > MaxHealth) Health = MaxHealth;
	}

	if (GetCharacterMovement()->IsFalling() && CoyoteTimeRemaining > 0.f) CoyoteTimeRemaining -= DeltaTime;
}

bool AMainCharacter::JumpIsAllowedInternal() const
{
	// Ensure that the CharacterMovement state is valid
	bool bJumpIsAllowed = GetCharacterMovement()->CanAttemptJump();

	if (bJumpIsAllowed)
	{
		// Ensure JumpHoldTime and JumpCount are valid.
		if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
		{
			if (JumpCurrentCount == 0
				&& GetCharacterMovement()->IsFalling()
				&& !Cast<UModularCharacterMovementComponent>(GetCharacterMovement())->NavModeAllowsFirstAirJump()
				&& CoyoteTimeRemaining <= 0.f)
			{
				bJumpIsAllowed = JumpCurrentCount + 1 < JumpMaxCount;
			}
			else
			{
				bJumpIsAllowed = JumpCurrentCount < JumpMaxCount;
			}
		}
		else
		{
			// Only consider JumpKeyHoldTime as long as:
			// A) The jump limit hasn't been met OR
			// B) The jump limit has been met AND we were already jumping
			const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
			bJumpIsAllowed = bJumpKeyHeld &&
				((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
		}
	}

	return bJumpIsAllowed;
}

void AMainCharacter::TakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                   AController* InstigatedBy, AActor* DamageCauser)
{
	Health -= Damage;
}
