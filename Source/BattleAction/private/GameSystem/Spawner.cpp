// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystem/Spawner.h"
#include "Monster/Monster.h"
#include "SubSystems/ActorPoolManager.h"
#include "ActorPool/ActorPool.h"


class UNavigationSystemV1;

USpawner::USpawner() :
	m_bUseSequentialSpawn(true),
	m_SpawnInterval(1.0f),
	m_MaxSpawnCount(1),
	m_CurrentSpawnCount(0),
	m_GridRows(4),
	m_GridColumns(3),
	m_GridSpacing(200.0f)
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

void USpawner::spawnMonstersInGrid() const
{
	const FVector spawnerLocation = GetComponentLocation();

	UActorPoolManager* poolManager = GetWorld()->GetGameInstance()->GetSubsystem<UActorPoolManager>();
	check(poolManager != nullptr);

	AActorPool* actorPool = poolManager->GetActorPool();
	check(actorPool != nullptr);

	int32 spawnedCount = 0;

	// 행/열 격자 패턴으로 스폰 (Spawner 위치 = 0,0)
	for (int32 row = 0; row < m_GridRows; ++row)
	{
		for (int32 col = 0; col < m_GridColumns; ++col)
		{
			// Spawner 위치에서 X축으로 row만큼, Y축으로 col만큼 오프셋
			const FVector offset = FVector(row * m_GridSpacing, col * m_GridSpacing, 0.0f);
			const FVector spawnLocation = spawnerLocation + offset;

			AMonster* monster = actorPool->SpawnActor<AMonster>(m_MonsterClass, spawnLocation);
			if (monster != nullptr)
			{
				++spawnedCount;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("USpawner::spawnMonstersInGrid - Spawned %d monsters (%d rows x %d columns)"),
		spawnedCount, m_GridRows, m_GridColumns);
}






