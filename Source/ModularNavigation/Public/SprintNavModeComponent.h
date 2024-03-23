// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavModeComponent.h"
#include "SprintNavModeComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARNAVIGATION_API USprintNavModeComponent : public UNavModeComponent
{
	GENERATED_BODY()

public:
	USprintNavModeComponent();

	// main
	/** The amount of increased speed to apply when sprinting (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sprint Nav Mode")
	float SpeedIncrease;
	
	virtual bool CanBegin() override;
	virtual void Begin() override;
	virtual void Exec(float DeltaTime) override;
	virtual void End() override;
};
