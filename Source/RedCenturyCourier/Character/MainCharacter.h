// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularNavigationCharacter.h"
#include "MainCharacter.generated.h"

// DOCSONLY: namespace Character {

/** Augments UE's character pawn, with additional functionality: jump whilst crouched, collect items. */
UCLASS()
class REDCENTURYCOURIER_API AMainCharacter : public AModularNavigationCharacter// , public ICollectorInterface
{
	GENERATED_BODY()
public:
	AMainCharacter(const FObjectInitializer& ObjectInitializer);
	// UFUNCTION()
	// virtual bool Collect(TScriptInterface<ICollectableInterface> Collectable) override;
	// /** Delegate to trigger whenever an item (that possesses CollectableComponent) is collected by this actor. */
	// UPROPERTY(BlueprintReadWrite, Category = "Main Character")
	// FOnCollection OnCollection;
	/** Health. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Main Character")
	float Health;
	/** Maximum health. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Main Character")
	float MaxHealth;
	/** Health Recovery rate. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Main Character")
	float HealthRecovery;
	/** The time after walking off a ledge, in seconds, wherein the character will still be allowed to jump. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 0, UIMin = 0), Category="Main Character")
	float CoyoteTime;
	/** A map of damage types, to the ratio of reduction to apply to the incoming damage (e.g. 0.2 means take 20% less damage). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Main Character")
	TMap<TSubclassOf<UDamageType>, float> DamageResistances;
	/** @returns UPlayerController for this Game Character. */
	UFUNCTION(BlueprintPure, Category = "Main Character")
	APlayerController* GetPlayerController() const;
	/** @returns whether the current movement input faces forwards */
	UFUNCTION(BlueprintPure, Category = "Main Character")
	bool IsInputForwards(const float maxAngle = 45.f) const;
	
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
	virtual void CheckJumpInput(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;
protected:
	virtual void BeginPlay() override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	float CoyoteTimeRemaining;
	// Replace Epic Games' implementation so that we can add coyote time
	// ReSharper disable once CppHidingFunction
	bool JumpIsAllowedInternal() const;
private:
	UFUNCTION()
	void TakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
};
// DOCSONLY: }
