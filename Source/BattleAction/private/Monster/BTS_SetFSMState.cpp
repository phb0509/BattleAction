// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BTS_SetFSMState.h"
#include "Monster/Monster.h"
#include "AIController.h"

#include "Utility/EnumTypes.h"


UBTS_SetFSMState::UBTS_SetFSMState() :
	m_EnumClassName(TEXT("")),
	m_SelectedFSMStateName(TEXT("")),
	m_FSMStateIndex(0)
{
	bNotifyBecomeRelevant = true;
	NodeName = TEXT("SetFSMState");
}

void UBTS_SetFSMState::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	AMonster* owner = Cast<AMonster>(OwnerComp.GetAIOwner()->GetPawn());
	check(owner != nullptr);
	
	owner->SetFSMState(m_FSMStateIndex);
}
