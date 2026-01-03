// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SignificanceManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BATTLEACTION_API USignificanceManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USignificanceManagerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Significance 값이 변경될 때 호출되는 함수
	void OnSignificanceChanged(float newSignificance);
	
public:
	// 최대 고려 거리 (센티미터). 이 거리 이상이면 Significance = 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Significance")
	float m_MawDistance = 2000.0f;

	// Low Significance일 때 AI를 일시정지할지 여부 (전투 중이 아닐 때만)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Significance")
	bool m_bPauseAIWhenLow = false;

	// High Significance 임계값 (0.6 이상)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Significance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float m_HighSignificanceThreshold = 0.6f;

	// Medium Significance 임계값 (0.3 이상)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Significance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float m_MediumSignificanceThreshold = 0.3f;


};
