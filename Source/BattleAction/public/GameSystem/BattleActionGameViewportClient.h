// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "BattleActionGameViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEACTION_API UBattleActionGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	
};
