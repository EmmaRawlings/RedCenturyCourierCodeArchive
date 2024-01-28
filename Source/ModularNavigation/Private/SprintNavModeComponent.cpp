// Fill out your copyright notice in the Description page of Project Settings.


#include "SprintNavModeComponent.h"

#include "DashNavModeComponent.h"
#include "ModularCharacterMovementComponent.h"
#include "ModularNavigationCharacter.h"


USprintNavModeComponent::USprintNavModeComponent()
{
	// Defaults
	SpeedIncrease = 500.f;
}

bool USprintNavModeComponent::CanBegin()
{
	TArray<UDashNavModeComponent*> DashNavModes;
	GetCharacter()->GetComponents<UDashNavModeComponent*>(DashNavModes);

	for (const UDashNavModeComponent* DashNavMode : DashNavModes)
	{
		if (DashNavMode->IsModeActive()) return false;
	}
	
	return GetCharacterMovement()->IsMovingOnGround();
}

void USprintNavModeComponent::Begin()
{
	Super::Begin();

	FMovementParameterContext MPC = NeutralContext();
	MPC.MaxSpeed += SpeedIncrease;
	GetCharacterMovement()->UseMovementParameterContext(MPC);
}

void USprintNavModeComponent::Exec(float DeltaTime)
{
	Super::Exec(DeltaTime);
}

void USprintNavModeComponent::End()
{
	GetCharacterMovement()->UnsetMovementParameterContext();
	
	Super::End();
}

