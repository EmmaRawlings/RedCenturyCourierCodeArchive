// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavModeComponent.h"
#include "SlideNavModeComponent.generated.h"


class MODULARNAVIGATION_API FSlideNavModeNetworkSavedData : public FNavModeNetworkSavedData
{
public:
	float ImpulseRemaining;
	FVector_NetQuantize10 LastVelocity;
	bool bLastWasAirSlide;

	virtual void Clear() override;
	virtual void RecordPostUpdate(ACharacter* Char, UNavModeComponent* NavMode, FSavedMove_Character::EPostUpdateMode PostUpdateMode) override;
	virtual void Serialize(FArchive& Ar, UPackageMap* PackageMap) override;
	virtual void Fill(ACharacter* Char, UNavModeComponent* NavMode) const override;
	virtual bool CheckForError(ACharacter* Char, UNavModeComponent* NavMode) const override;
protected:
	virtual FNavModeNetworkSavedData* NewInstance() const override;
	virtual void CopyFrom(const FNavModeNetworkSavedData* Original) override;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARNAVIGATION_API USlideNavModeComponent : public UNavModeComponent
{
	GENERATED_BODY()

	friend FSlideNavModeNetworkSavedData;
	
public:
	USlideNavModeComponent();

	// main
	/** How much friction to apply during a slide on any surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Slide Navigation Mode")
	float Friction;
	/** The acceleration to apply when inputting movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Slide Navigation Mode")
	float MovementAcceleration;
	/** The speed threshold to pass to be able to enter a slide. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Slide Navigation Mode")
	float EnterThreshold;
	/** The speed threshold which exits the slide, when the speed falls below it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Slide Navigation Mode")
	float ExitThreshold;
private:
	/** Impulse to apply when entering a slide whilst walking. */
	UPROPERTY(EditAnywhere, BlueprintGetter=GetImpulse, BlueprintSetter=SetImpulse, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Slide Navigation Mode")
	float Impulse;
public:
	/** Impulse is consumed when applied, and recovers over time. This determines the time the impulse takes to recover. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", UIMin = "0.0"),
		Category="EsperFPS | Slide Navigation Mode")
	float ImpulseRecoveryTime;
	/** Maximum total speed to boost the character up to when applying impulse on entering a slide. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Slide Navigation Mode")
	float MaxImpulseSpeed;
	
	UFUNCTION(BlueprintPure, BlueprintInternalUseOnly)
	float GetImpulse() const;
	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
	void SetImpulse(const float InImpulse);
	
protected:
	virtual void BeginPlay() override;
public:
	virtual bool CanBegin() override;
	virtual void Begin() override;
	virtual void Exec(float DeltaTime) override;
	virtual bool ShouldEnd() override;
	virtual void End() override;
	virtual void Idle(float DeltaTime) override;
	virtual bool AllowsAirLedgeStep() override;
	virtual FNavModeNetworkSavedData* NewNetworkSavedData() const override
	{
		return new FSlideNavModeNetworkSavedData();
	}
	virtual FString GetDebugInfo() override;
private:
	float ImpulseRemaining;
	FVector LastVelocity;
	bool bLastWasAirSlide;

};
