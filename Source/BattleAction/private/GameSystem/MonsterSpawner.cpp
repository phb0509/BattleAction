// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystem/MonsterSpawner.h"
#include "GameSystem/MonsterSpawnerManager.h"

UMonsterSpawner::UMonsterSpawner()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void UMonsterSpawner::StartSpawn()
{
	
}

#if WITH_EDITOR
void UMonsterSpawner::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	UE_LOG(LogTemp, Warning, TEXT("UMonsterSpawner :: OnComponentDestroyed"));
	
	if (m_OwnerManager.IsValid())
	{
		m_OwnerManager->RemoveMonsterSpawner(*this->GetName());
	}
}
#endif

