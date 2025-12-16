// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystem/SpawnerManager.h"
#include "Components/BoxComponent.h"
#include "PlayableCharacter/PlayableCharacter.h"
#include "Monster/Monster.h"
#include "SubSystems/ActorPoolManager.h"
#include "ActorPool/ActorPool.h"
#include "GameSystem/Spawner.h"


ASpawnerManager::ASpawnerManager() :
	m_bHasTriggered(false)
{
	PrimaryActorTick.bCanEverTick = false;

	m_TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("m_TriggerVolume"));
	RootComponent = m_TriggerVolume;

	m_TriggerVolume->SetBoxExtent(FVector(200.f, 200.f, 100.f));
	m_TriggerVolume->SetHiddenInGame(true);
	m_TriggerVolume->ShapeColor = FColor::Green;
	m_TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ASpawnerManager::OnBeginOverlap);
}

void ASpawnerManager::BeginPlay()
{
	Super::BeginPlay();

	collectSpawners();
	preallocateMonsterPool();
}

void ASpawnerManager::collectSpawners()
{
	TArray<USpawner*> spawnerComponents;
	GetComponents<USpawner>(spawnerComponents);

	m_Spawners.Reserve(spawnerComponents.Num());

	for (USpawner* spawner : spawnerComponents)
	{
		m_Spawners.Add(spawner);
	}

	UE_LOG(LogTemp, Log, TEXT("ASpawnerManager::collectSpawners - Found %d spawner(s)"), m_Spawners.Num());
}

void ASpawnerManager::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (m_bHasTriggered)
	{
		return;
	}
	
	m_bHasTriggered = true;
	
	UE_LOG(LogTemp, Warning, TEXT("ASpawnerManager::OnBeginOverlap - Player detected, starting spawn"));

	startAllSpawners();
}

void ASpawnerManager::preallocateMonsterPool()
{
	check(!m_Spawners.IsEmpty());

	UActorPoolManager* poolManager = GetWorld()->GetGameInstance()->GetSubsystem<UActorPoolManager>();
	check(poolManager != nullptr);

	AActorPool* actorPool = poolManager->GetActorPool();
	check(actorPool != nullptr);

	TMap<TSubclassOf<AMonster>, int32> monsterRequirements;

	for (const auto& spawner : m_Spawners)
	{
		check(spawner != nullptr);

		TSubclassOf<AMonster> monsterClass = spawner->GetMonsterClass();
		check(monsterClass != nullptr);

		int32 count = spawner->GetMaxSpawnCount();
		int32& currentCount = monsterRequirements.FindOrAdd(monsterClass, 0);
		
		currentCount += count;
	}

	for (const auto& pair : monsterRequirements)
	{
		actorPool->CreateActorPool(pair.Key, pair.Value);
		UE_LOG(LogTemp, Log, TEXT("ASpawnerManager::preallocateMonsterPool - Preallocated %d monsters of class %s"),
			pair.Value, *pair.Key->GetName());
	}
}

void ASpawnerManager::startAllSpawners()
{
	for (const auto& spawner : m_Spawners)
	{
		check(spawner != nullptr);
		spawner->StartSpawn();
	}
}


