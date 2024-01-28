// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameInstance.h"

#include "UserSettings.h"
#include "Kismet/GameplayStatics.h"

UMainGameInstance::UMainGameInstance()
{
	UserSettingsManager = CreateDefaultSubobject<UUserSettingsManager>(TEXT("SettingsManager"));
}

void UMainGameInstance::Init()
{
	Super::Init();

	UserSettingsManager->Init();
}

UMainGameInstance* UMainGameInstance::GetGameInstance(const UObject* WorldContextObject)
{
	return Cast<UMainGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
}
