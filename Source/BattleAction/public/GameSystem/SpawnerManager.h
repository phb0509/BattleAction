// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnerManager.generated.h"


class UBoxComponent;
class AMonster;
class USpawner;

UCLASS()
class BATTLEACTION_API ASpawnerManager : public AActor
{
	GENERATED_BODY()

public:
	ASpawnerManager();

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
					   bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;

private:
	void collectSpawners();
	void preallocateMonsterPool();
	void startAllSpawners();

public:
	UPROPERTY(EditAnywhere, Category = "Trigger")
	TObjectPtr<UBoxComponent> m_TriggerVolume;

private:
	TArray<TObjectPtr<USpawner>> m_Spawners;
	bool m_bHasTriggered;

};
