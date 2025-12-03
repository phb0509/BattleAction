// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawnerManager.generated.h"

class UBoxComponent;
class UMonsterSpawner;
class AMonster;


USTRUCT(BlueprintType)
struct FSpawnerSavedData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AMonster> MonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnInterval = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 MaxSpawnCount = 1;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	FVector RelativeLocation = FVector::ZeroVector;
};

UCLASS()
class BATTLEACTION_API AMonsterSpawnerManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AMonsterSpawnerManager();
	
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override; // true 리턴 시, 에디터에서 틱호출.
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostLoad() override;
	void SaveSpawnerDataToDisk();
	
	
	// MonsterSpawner가 파괴될 때 호출
	// 특히 에디터 OutLiner 하단 디테일패널의 상속트리에서 삭제할 때, m_MonsterSpawners에 바로 반영하기 위함.
	// m_MonsterSpawners에는 항상 유효한 MonsterSpawner만 있어야 함. 
	void RemoveMonsterSpawner(const FName& componentName); 
#endif
	
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
					   bool bFromSweep, const FHitResult& SweepResult);

private:
	void spawnMonsters();
	
#if WITH_EDITOR
	void handleArrayAdd(FPropertyChangedEvent& PropertyChangedEvent);
	void handleArrayRemove(FPropertyChangedEvent& PropertyChangedEvent);
	void handleArrayClear();

	void refreshMonsterSpawners(); // 유효하지 않은 컴포넌트 TArray에서 삭제.
	void refreshEditorView() const;// 에디터 UI 업데이트.


#endif

	
public:	
	// 트리거 볼륨 (박스 또는 캡슐)
	UPROPERTY(EditAnywhere, Category = "Trigger")
	TObjectPtr<UBoxComponent> TriggerVolume;
	
	// 스폰할 몬스터 클래스 리스트
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<TObjectPtr<UMonsterSpawner>> m_MonsterSpawners;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<FSpawnerSavedData> SpawnerSavedDataArray;
	
	TArray<FTransform> tempSavedTransforms;
};
