#include "ModularCharacterMovementReplication.h"
#include "ModularCharacterMovementComponent.h"

void FCharacterNetworkModularMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove,
                                                                 ENetworkMoveType MoveType)
{
	FCharacterNetworkMoveData::ClientFillNetworkMoveData(ClientMove, MoveType);

	const FSavedModularMove_Character& ModularClientMove = static_cast<const FSavedModularMove_Character&>(ClientMove);
	
	TArray<uint8> NavModeKeys;
	ModularClientMove.NavModePilotData.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		NavModePilotData.Add(NavModeKeyEntry, ModularClientMove.NavModePilotData[NavModeKeyEntry]->Clone());
	}
	SavedNavModeKey = ModularClientMove.SavedNavModeKey;
	ModularClientMove.NavModeSavedData.GetKeys(NavModeKeys);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		NavModeSavedData.Add(NavModeKeyEntry, ModularClientMove.NavModeSavedData[NavModeKeyEntry]->Clone());
	}
}

bool FCharacterNetworkModularMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar,
	UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	const bool bIsSaving = Ar.IsSaving();
	const bool bSuccessFromSuper = FCharacterNetworkMoveData::Serialize(CharacterMovement, Ar, PackageMap, MoveType);
	
	/* NOTE: Normal UE move data does not appear to ever exceed 900 bits, nav mode data should try to keep as small
	 * as possible, hopefully never ever exceeding 1000 bits (<500 sounds reasonable, even for more complex modes).
	 * See CharacterMovementComponent.cpp->FCharacterNetworkSerializationPackedBits::NetSerialize for bit max check. */
	UModularCharacterMovementComponent* ModularCharacterMovementComponent = Cast<UModularCharacterMovementComponent>(&CharacterMovement);
	TArray<uint8> NavModeKeys;
	ModularCharacterMovementComponent->NavModeByKey.GetKeys(NavModeKeys);
	
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		if (!NavModePilotData.Contains(NavModeKeyEntry))
			NavModePilotData.Add(NavModeKeyEntry, ModularCharacterMovementComponent->NavModeByKey[NavModeKeyEntry]->NewNetworkPilotData());
		NavModePilotData[NavModeKeyEntry]->Serialize(Ar, PackageMap);
	}
	SerializeOptionalValue<uint8>(bIsSaving, Ar, SavedNavModeKey, 0);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		if (!NavModeSavedData.Contains(NavModeKeyEntry))
			NavModeSavedData.Add(NavModeKeyEntry, ModularCharacterMovementComponent->NavModeByKey[NavModeKeyEntry]->NewNetworkSavedData());
		NavModeSavedData[NavModeKeyEntry]->Serialize(Ar, PackageMap);
	}
	
	return bSuccessFromSuper && !Ar.IsError();
}

void FCharacterModularMoveResponseDataContainer::ServerFillResponseData(
	const UCharacterMovementComponent& CharacterMovement, const FClientAdjustment& PendingAdjustment)
{
	FCharacterMoveResponseDataContainer::ServerFillResponseData(CharacterMovement, PendingAdjustment);

	const FNetworkPredictionData_Server_ModularCharacter* ServerData =
		static_cast<FNetworkPredictionData_Server_ModularCharacter*>(CharacterMovement.GetPredictionData_Server_Character());
	check(ServerData);
	if (!ServerData->PendingAdjustment.bAckGoodMove)
	{
		ClientAdjustmentNavModeKey = ServerData->PendingAdjustmentNavModeKey;
		TArray<uint8> NavModeKeys;
		ServerData->PendingAdjustmentNavModeData.GetKeys(NavModeKeys);
		for (const uint8 NavModeKeyEntry : NavModeKeys)
		{
			ClientAdjustmentNavModeData.Add(NavModeKeyEntry, ServerData->PendingAdjustmentNavModeData[NavModeKeyEntry]->Clone());
		}
	}
}

bool FCharacterModularMoveResponseDataContainer::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar,
	UPackageMap* PackageMap)
{
	const bool bIsSaving = Ar.IsSaving();
	const bool bSuccessFromSuper = FCharacterMoveResponseDataContainer::Serialize(CharacterMovement, Ar, PackageMap);
	
	UModularCharacterMovementComponent* ModularCharacterMovementComponent = Cast<UModularCharacterMovementComponent>(&CharacterMovement);
	TArray<uint8> NavModeKeys;
	ModularCharacterMovementComponent->NavModeByKey.GetKeys(NavModeKeys);
	
	SerializeOptionalValue<uint8>(bIsSaving, Ar, ClientAdjustmentNavModeKey, 0);
	for (const uint8 NavModeKeyEntry : NavModeKeys)
	{
		if (!ClientAdjustmentNavModeData.Contains(NavModeKeyEntry))
			ClientAdjustmentNavModeData.Add(NavModeKeyEntry, ModularCharacterMovementComponent->NavModeByKey[NavModeKeyEntry]->NewNetworkSavedData());
		ClientAdjustmentNavModeData[NavModeKeyEntry]->Serialize(Ar, PackageMap);
	}
	
	return bSuccessFromSuper && !Ar.IsError();
}
