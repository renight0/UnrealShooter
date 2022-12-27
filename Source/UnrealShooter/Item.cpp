// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"


AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_itemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("_itemMesh"));

	SetRootComponent(_itemMesh);

	_collisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("_collisionBox"));
	_collisionBox->SetupAttachment(_itemMesh);

}


void AItem::BeginPlay()
{
	Super::BeginPlay();
	
}


void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

