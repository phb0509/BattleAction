// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "MainPlayerController.generated.h"


UCLASS()
class BATTLEACTION_API AMainPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AMainPlayerController();

	virtual void BeginPlay() override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void SetupInputComponent() override;
	
	
	void OpenEnvironmentSettingsState();

private:
	FGenericTeamId m_TeamID;
	
};
