// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/Monster.h"
#include "TestMonster.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEACTION_API ATestMonster : public AMonster
{
	GENERATED_BODY()
	
public:
	ATestMonster();
	
	virtual void BeginPlay() override;
	virtual void Activate() override;
	
private:
	void initAssets();
	
private:

};
