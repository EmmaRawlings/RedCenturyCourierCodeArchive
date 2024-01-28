// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavModeComponent.h"
#include "LedgeVaultNavModeComponent.generated.h"

// TODO feel like there should be pilot data...
// class MODULARNAVIGATION_API FLedgeVaultNavModeNetworkPilotData : public FNavModeNetworkPilotData
// {
// public:
// 	virtual void Clear() override;
// 	virtual void RecordPreflight(ACharacter* Char, UNavModeComponent* NavMode, float InDeltaTime) override;
// 	virtual void Serialize(FArchive& Ar, UPackageMap* PackageMap) override;
// 	virtual void Fill(ACharacter* Char, UNavModeComponent* NavMode) const override;
// protected:
// 	virtual FNavModeNetworkPilotData* NewInstance() const override;
// 	virtual void CopyFrom(const FNavModeNetworkPilotData* Original) override;
// };

class MODULARNAVIGATION_API FLedgeVaultNavModeNetworkSavedData : public FNavModeNetworkSavedData
{
public:
	float Progress;
	FVector_NetQuantize100 From;
	FVector_NetQuantize100 To;

	virtual void Clear() override;
	virtual void RecordPostUpdate(ACharacter* Char, UNavModeComponent* NavMode, FSavedMove_Character::EPostUpdateMode PostUpdateMode) override;
	virtual void Serialize(FArchive& Ar, UPackageMap* PackageMap) override;
	virtual void Fill(ACharacter* Char, UNavModeComponent* NavMode) const override;
	virtual bool CheckForError(ACharacter* Char, UNavModeComponent* NavMode) const override;
protected:
	virtual FNavModeNetworkSavedData* NewInstance() const override;
	virtual void CopyFrom(const FNavModeNetworkSavedData* Original) override;
};

constexpr float LEDGE_VAULT_SEARCH_INCREMENTS = 5.f;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARNAVIGATION_API ULedgeVaultNavModeComponent : public UNavModeComponent
{
	GENERATED_BODY()

	/*friend FLedgeVaultNavModeNetworkPilotData;*/ friend FLedgeVaultNavModeNetworkSavedData;

public:
	ULedgeVaultNavModeComponent();

	// main
	/** The highest point, relative to the character's feet, where a ledge is checked to vaulted.
	 *	Recommended this be set to be above the character's head somewhat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Ledge Vault Navigation Mode")
	float MaxHeight;
	/** The lowest point, relative to the character's feet, where a ledge is checked to vaulted.
	 *	Recommended this be set to around half of the character's height. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Ledge Vault Navigation Mode")
	float MinHeight;
	/** How far forwards the character can reach to grab a ledge.
	 *	Recommended this be about as far as the character's arm span. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Ledge Vault Navigation Mode")
	float ForwardsReach;
	/** The maximum incline of the ledge which the character can vault onto. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Ledge Vault Navigation Mode")
	float MaxSlopeAngle;
	/** How long, in seconds, it takes to complete a ledge grab. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EsperFPS | Ledge Vault Navigation Mode")
	float Duration;
	
	virtual bool CanBegin() override;
	virtual void Begin() override;
	virtual void Exec(float DeltaTime) override;
	virtual bool ShouldEnd() override;
	virtual void End() override;
	virtual void Idle(float DeltaTime) override;
	virtual bool AllowsAirLedgeStep() override;
	virtual FNavModeNetworkSavedData* NewNetworkSavedData() const override
	{
		return new FLedgeVaultNavModeNetworkSavedData();
	}
	virtual FString GetDebugInfo() override;
private:
	float Progress = -1.f;
	FVector From;
	FVector To;
	bool DetectLedgeToVault(FVector& OutTo) const;
};
