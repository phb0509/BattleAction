// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_SetFSMState.generated.h"



UCLASS()
class BATTLEACTION_API UBTS_SetFSMState : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_SetFSMState();
	
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	FORCEINLINE FText GetFSMStateName() const { return FText::FromString(m_SelectedFSMStateName); } 
	FORCEINLINE void SetFSMStateName(const FString& stateName) { m_SelectedFSMStateName = stateName; }

	FORCEINLINE uint8 GetFSMStateIndex() const { return m_FSMStateIndex; } 
	FORCEINLINE void SetFSMStateIndex(const uint8 stateIndex){ m_FSMStateIndex = stateIndex; }

	FORCEINLINE FText GetEnumClassName() const { return FText::FromString(m_EnumClassName); }
	FORCEINLINE void SetEnumClassName(const FString& enumClassName) { m_EnumClassName = enumClassName; }

private:
	UPROPERTY(VisibleAnywhere, meta = (HideInDetailPanel))
	FString m_EnumClassName;
	
	UPROPERTY(VisibleAnywhere, meta = (HideInDetailPanel))
	FString m_SelectedFSMStateName;

	UPROPERTY(VisibleAnywhere, meta = (HideInDetailPanel))
	uint8 m_FSMStateIndex;
};
