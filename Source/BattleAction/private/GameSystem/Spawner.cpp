// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystem/Spawner.h"

#include "NavigationSystem.h"
#include "Monster/Monster.h"
#include "SubSystems/ActorPoolManager.h"
#include "ActorPool/ActorPool.h"


class UNavigationSystemV1;

USpawner::USpawner() :
	m_SpawnInterval(1.0f),
	m_MaxSpawnCount(1),
	m_CurrentSpawnCount(0)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!m_MonsterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("USpawner::BeginPlay - MonsterClass is not set! Owner: %s"),
			*GetOwner()->GetName());
	}
}

void USpawner::StartSpawn()
{
	if (!m_MonsterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("USpawner::StartSpawn - Cannot spawn, MonsterClass is null!"));
		return;
	}

	if (m_bUseSequentialSpawn)
	{
		startSequentialSpawn();
	}
	else
	{
		spawnMonstersInGrid();
	}
}

void USpawner::startSequentialSpawn()
{
	m_CurrentSpawnCount = 0;

	GetWorld()->GetTimerManager().SetTimer(
		m_SpawnTimerHandle,
		this,
		&USpawner::spawnMonster,
		m_SpawnInterval,
		true,
		0.0f
	);
}

void USpawner::spawnMonster()
{
	if (m_CurrentSpawnCount >= m_MaxSpawnCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(m_SpawnTimerHandle);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Spawner World Location: %s"),
	  *GetComponentLocation().ToString());

	
	UActorPoolManager* poolManager = GetWorld()->GetGameInstance()->GetSubsystem<UActorPoolManager>();
	check(poolManager != nullptr);
	
	AActorPool* actorPool = poolManager->GetActorPool();
	check(actorPool != nullptr);
	
	AMonster* monster = actorPool->SpawnActor<AMonster>(m_MonsterClass, this->GetComponentLocation());
	check(monster != nullptr);

	++m_CurrentSpawnCount;
}

void USpawner::spawnMonstersInGrid()
{
	const FVector spawnerLocation = GetComponentLocation();

	UActorPoolManager* poolManager = GetWorld()->GetGameInstance()->GetSubsystem<UActorPoolManager>();
	check(poolManager != nullptr);

	AActorPool* actorPool = poolManager->GetActorPool();
	check(actorPool != nullptr);

	int32 spawnedCount = 0;

	// 격자 패턴으로 스폰
	for (float x = -m_GridRangeX; x <= m_GridRangeX; x += m_GridSpacing)
	{
		for (float y = -m_GridRangeY; y <= m_GridRangeY; y += m_GridSpacing)
		{
			// 스포너 중심 위치는 제외 (플레이어가 있을 위치)
			if (FMath::IsNearlyZero(x) && FMath::IsNearlyZero(y))
			{
				continue;
			}

			FVector spawnLocation = spawnerLocation + FVector(x, y, 0.0f);

			AMonster* monster = actorPool->SpawnActor<AMonster>(m_MonsterClass, spawnLocation);
			if (monster)
			{
				++spawnedCount;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("USpawner::spawnMonstersInGrid - Spawned %d monsters in grid pattern"), spawnedCount);
}






