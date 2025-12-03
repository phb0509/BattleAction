// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/TestMonster/TestMonster.h"
#include "SubSystems/UIManager.h"

ATestMonster::ATestMonster()
{
	PrimaryActorTick.bCanEverTick = true; 
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	Tags.Add(TEXT("TestMonster"));
	
}

void ATestMonster::BeginPlay()
{
	Super::BeginPlay();
}

void ATestMonster::Activate()
{
	Super::Activate();
}

