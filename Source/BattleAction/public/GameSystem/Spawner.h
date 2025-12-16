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

	// true: 순차 스폰 (기존), false: 격자 동시 스폰 (신규)
	UPROPERTY(EditAnywhere, Category = "Spawn")
	bool m_bUseSequentialSpawn = true;

	// === 순차 스폰 설정 ===
	UPROPERTY(EditAnywhere, Category = "Spawn|Sequential", meta = (EditCondition = "m_bUseSequentialSpawn", EditConditionHides))
	float m_SpawnInterval;

	UPROPERTY(EditAnywhere, Category = "Spawn|Sequential", meta = (EditCondition = "m_bUseSequentialSpawn", EditConditionHides))
	int32 m_MaxSpawnCount;

	FTimerHandle m_SpawnTimerHandle;
	int32 m_CurrentSpawnCount;

	// === 격자 스폰 설정 ===
	UPROPERTY(EditAnywhere, Category = "Spawn|Grid", meta = (EditCondition = "!m_bUseSequentialSpawn", EditConditionHides))
	float m_GridRangeX = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Grid", meta = (EditCondition = "!m_bUseSequentialSpawn", EditConditionHides))
	float m_GridRangeY = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Grid", meta = (EditCondition = "!m_bUseSequentialSpawn", EditConditionHides))
	float m_GridSpacing = 100.0f;

	void startSequentialSpawn();
	void spawnMonster();
	void spawnMonstersInGrid();

};
