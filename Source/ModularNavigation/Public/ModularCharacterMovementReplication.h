
#pragma once

#include "NavModeComponent.h"
#include "GameFramework/CharacterMovementReplication.h"

struct MODULARNAVIGATION_API FCharacterNetworkModularMoveData : public FCharacterNetworkMoveData
{
public:
	FCharacterNetworkModularMoveData()
		 : SavedNavModeKey(0)
	{
	}

	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;

	TMap<uint8, FNavModeNetworkPilotData*> NavModePilotData;
	uint8 SavedNavModeKey;
	TMap<uint8, FNavModeNetworkSavedData*> NavModeSavedData;
};

struct MODULARNAVIGATION_API FCharacterNetworkModularMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
public:
	FCharacterNetworkModularMoveDataContainer()
	{
		NewMoveData		= new FCharacterNetworkModularMoveData();
		PendingMoveData	= new FCharacterNetworkModularMoveData();
		OldMoveData		= new FCharacterNetworkModularMoveData();
	}
};

struct MODULARNAVIGATION_API FCharacterModularMoveResponseDataContainer : public FCharacterMoveResponseDataContainer
{
public:
	virtual void ServerFillResponseData(const UCharacterMovementComponent& CharacterMovement, const FClientAdjustment& PendingAdjustment) override;
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap) override;

	uint8 ClientAdjustmentNavModeKey;
	TMap<uint8, FNavModeNetworkSavedData*> ClientAdjustmentNavModeData;
};
