// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MonsterSpawner.generated.h"


class AMonster;
class AMonsterSpawnerManager;




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BATTLEACTION_API UMonsterSpawner : public USceneComponent
{


private:
	GENERATED_BODY()

public:	
	UMonsterSpawner();

	void StartSpawn();
	
#if WITH_EDITOR
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	void SetOwnerManager(AMonsterSpawnerManager* InManager) { m_OwnerManager = InManager; }
#endif
	
protected:
	virtual void BeginPlay() override;
	

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AMonster> m_MonsterClass;

	UPROPERTY(EditAnywhere) 
	float m_SpawnInterval; // 스폰 주기 (ex 0.25초마다 스폰)

	UPROPERTY(EditAnywhere)
	int32 m_MaxSpawnCount; // 최대 생성 개수

	TWeakObjectPtr<AMonsterSpawnerManager> m_OwnerManager;
};
