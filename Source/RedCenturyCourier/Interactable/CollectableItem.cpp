// Fill out your copyright notice in the Description page of Project Settings.


#include "CollectableItem.h"

#include "Components/BoxComponent.h"


ACollectableItem::ACollectableItem()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionShape = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionShape"));
	RootComponent = CollisionShape;
	// Mesh->AlwaysLoadOnClient = true;
	// Mesh->AlwaysLoadOnServer = true;
	// Mesh->bOwnerNoSee = false;
	// Mesh->bCastDynamicShadow = true;
	// Mesh->bAffectDynamicIndirectLighting = true;
	// Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	// static FName MeshCollisionProfileName(TEXT("CharacterMesh"));
	// Mesh->SetCollisionProfileName(MeshCollisionProfileName);
	// Mesh->SetGenerateOverlapEvents(false);
	// Mesh->SetCanEverAffectNavigation(false);

	CollectableSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectableSphere->InitSphereRadius(25.f);
	static FName CollisionProfileName(TEXT("OverlapAllDynamic"));
	CollectableSphere->SetCollisionProfileName(CollisionProfileName);
	// CollectionSphere->CanCharacterStepUpOn = ECB_No;
	CollectableSphere->SetGenerateOverlapEvents(true);
	CollectableSphere->SetupAttachment(RootComponent);

	ItemType = TEXT("None");
	ItemName = TEXT("");
}

void ACollectableItem::BeginPlay()
{
	Super::BeginPlay();
}

void ACollectableItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

