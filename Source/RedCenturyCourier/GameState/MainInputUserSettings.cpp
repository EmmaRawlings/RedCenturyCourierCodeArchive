// Fill out your copyright notice in the Description page of Project Settings.


#include "MainInputUserSettings.h"

#include "MainGameInstance.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_InvalidMappingName, "InputUserSettings.FailureReasons.InvalidMappingName");

struct FAxisPropertiesHeader
{
	friend FArchive& operator<<(FArchive& Ar, FAxisPropertiesHeader& Header)
	{
		Ar << Header.MappingName;
		Ar << Header.AxisProperties;
		return Ar;
	}

	FName MappingName;
	FAxisProperties AxisProperties;
};

void UMainInputUserSettings::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (IsTemplate() || Ar.IsCountingMemory())
	{
		return;
	}
	
	TArray<FAxisPropertiesHeader> Headers;
	if (Ar.IsSaving())
	{
		for (const TPair<FName, FAxisProperties> PropertyPair : SavedAxisProperties)
		{
			FAxisPropertiesHeader Header =
				{
				/* .MappingName = */ PropertyPair.Key,
				/* .AxisProperties = */ PropertyPair.Value
				};

			Headers.Push(Header);
		}
	}
	
	Ar << Headers;

	if (Ar.IsLoading())
	{
		for (const FAxisPropertiesHeader& Header : Headers)
		{
			SavedAxisProperties.Add(Header.MappingName, Header.AxisProperties);
		}
	}
}

void UMainInputUserSettings::SetAxisProperties(const FName MappingName, const FAxisProperties& InProperties,
                                               FGameplayTagContainer& FailureReason)
{
	if (!MappingName.IsValid())
	{
		FailureReason.AddTag(TAG_InvalidMappingName);
		return;
	}

	SavedAxisProperties.Emplace(MappingName, InProperties);
	
	// TODO something like: OnKeyMappingUpdated(FoundMapping, InArgs, false);

	// TODO support deferred on settings change broadcast
	// if( InArgs.bDeferOnSettingsChangedBroadcast && !DeferredSettingsChangedTimerHandle.IsValid())
	// {
	// 	if(UWorld* World = GetWorld())
	// 	{
	// 		TWeakObjectPtr<UEnhancedInputUserSettings> WeakThis = this;
	// 		DeferredSettingsChangedTimerHandle = World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, 
	// 			[WeakThis]
	// 			{
	// 				if(WeakThis.IsValid())
	// 				{
	// 					WeakThis->OnSettingsChanged.Broadcast(WeakThis.Get());
	// 					WeakThis->DeferredSettingsChangedTimerHandle.Invalidate();
	// 				}
	// 			}));
	// 	}
	// }
	// else
	// {
		OnSettingsChanged.Broadcast(this);
	// }
}

bool UMainInputUserSettings::HasAxisProperties(const FName MappingName) const
{
	return SavedAxisProperties.Contains(MappingName);
}

FAxisProperties UMainInputUserSettings::GetAxisProperties(const FName MappingName) const
{
	if (HasAxisProperties(MappingName))
		return SavedAxisProperties[MappingName];

	const auto World = GetWorld();
	if (World == nullptr)
		return FAxisProperties();
	
	const UMainGameInstance* GameInstance = Cast<UMainGameInstance>(World->GetGameInstance());
	if (GameInstance == nullptr)
		return FAxisProperties();

	return GameInstance->GetDefaultAxisProperties(MappingName);
}
