// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ModularNavigationCharacter.generated.h"

UCLASS()
class MODULARNAVIGATION_API AModularNavigationCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AModularNavigationCharacter(const FObjectInitializer& ObjectInitializer);
	/** Allows the character to jump whilst crouched. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EsperFPS | Game Character")
	bool bCanJumpWhilstCrouched;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EsperFPS | Modular Movement")
	USceneComponent* CapsuleHeadRoot;
	
	virtual void CheckJumpInput(float DeltaTime) override;
protected:
	// Replace Epic Games' implementation so that we can allow first air jump for some nav modes
	// ReSharper disable once CppHidingFunction
	bool JumpIsAllowedInternal() const;
	/* Allow jumping whilst crouched. */
	virtual bool CanJumpInternal_Implementation() const override;
};
