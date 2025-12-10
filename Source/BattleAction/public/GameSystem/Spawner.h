// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Spawner.generated.h"


class AMonster;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BATTLEACTION_API USpawner : public USceneComponent
{
	GENERATED_BODY()

public:
	USpawner();

	void StartSpawn();

	FORCEINLINE TSubclassOf<AMonster> GetMonsterClass() const { return m_MonsterClass; }
	FORCEINLINE int32 GetMaxSpawnCount() const { return m_MaxSpawnCount; }

protected:
	virtual void BeginPlay() override;


private:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AMonster> m_MonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float m_SpawnInterval;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 m_MaxSpawnCount;

	FTimerHandle m_SpawnTimerHandle;
	int32 m_CurrentSpawnCount;

	void spawnMonster();

};
