// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularNavigationCharacter.h"

#include "ModularCharacterMovementComponent.h"


AModularNavigationCharacter::AModularNavigationCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UModularCharacterMovementComponent>(TEXT("CharMoveComp")))
{
	CapsuleHeadRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CapsuleHeadRoot"));
	CapsuleHeadRoot->SetupAttachment(GetRootComponent());
}

bool AModularNavigationCharacter::CanJumpInternal_Implementation() const
{
	if (bCanJumpWhilstCrouched)
		return JumpIsAllowedInternal();
	else
		return !bIsCrouched && JumpIsAllowedInternal();
}

void AModularNavigationCharacter::CheckJumpInput(float DeltaTime)
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
				&& !Cast<UModularCharacterMovementComponent>(GetCharacterMovement())->NavModeAllowsFirstAirJump())
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

bool AModularNavigationCharacter::JumpIsAllowedInternal() const
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
				&& !Cast<UModularCharacterMovementComponent>(GetCharacterMovement())->NavModeAllowsFirstAirJump())
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

