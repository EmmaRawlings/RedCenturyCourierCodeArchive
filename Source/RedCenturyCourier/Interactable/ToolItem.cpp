// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolItem.h"


AToolItem::AToolItem()
{
	PrimaryActorTick.bCanEverTick = true;
	bActionActive = false;
}

void AToolItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AToolItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

