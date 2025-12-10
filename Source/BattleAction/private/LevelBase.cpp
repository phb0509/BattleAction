// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelBase.h"
#include "SubSystems/DebugManager.h"


ALevelBase::ALevelBase()
{
	UE_LOG(LogTemp, Warning, TEXT("LevelBase :: Constructor"));
}

void ALevelBase::ToggleDebugMode()
{
	UDebugManager* debugManager = GetGameInstance()->GetSubsystem<UDebugManager>();
	check(debugManager != nullptr);

	debugManager->ToggleDebugMode();
}

