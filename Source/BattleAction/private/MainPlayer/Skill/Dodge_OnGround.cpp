// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayer/Skill/Dodge_OnGround.h"
#include "PlayableCharacter/PlayableCharacter.h"
#include "CharacterBase/AnimInstanceBase.h"
#include "Component/MainPlayerSkillComponent.h"
#include "MotionWarpingComponent.h"
#include "Utility/EnumTypes.h"

UDodge_OnGround::UDodge_OnGround() :
	m_DodgeMontage(nullptr),
	m_MoveDistance(400.0f),
	m_InvincibleTime(0.5f)
{
}

void UDodge_OnGround::Initialize()
{
	Super::Initialize();
	
	check(m_DodgeMontage != nullptr);
}

void UDodge_OnGround::Execute(const FInputInfos& inputInfos)
{
	Super::Execute(inputInfos);

	m_Owner->SetInvincible(true);

	m_Owner->GetWorldTimerManager().SetTimer(
		m_InvincibilityTimer,
		[this]()
		{
			m_Owner->SetInvincible(false);
		},
		m_InvincibleTime,
		false
	);

	UMainPlayerSkillComponent* ownerSkillComponent = CastChecked<UMainPlayerSkillComponent>(m_OwnerSkillComponent);
	ownerSkillComponent->SetSkillState(EMainPlayerSkillStates::Dodge_NonTargeting_OnGround);
	
	m_Owner->BreakCrowdControlState();
	m_Owner->Multicast_StopAllMontages(0.0f);

	const FRotator controlYawRotator(0.0f, inputInfos.controlYaw, 0.0f);
	const FVector controllerForwardVec = controlYawRotator.Vector();
	const FVector controllerRightVec = FRotationMatrix(controlYawRotator).GetUnitAxis(EAxis::Y);

	// const int32 directionIndex = m_Owner->GetDirectionIndexFromKeyInput(inputInfos.inputVertical, inputInfos.inputHorizontal);
	// const FVector worldKeyInputDirection = FRotator(0.0f, inputInfos.controlYaw + 45.0f * directionIndex, 0.0f).Vector();
	// const int32 localDirectionIndex = m_Owner->GetLocalDirectionIndex(worldKeyInputDirection);
	// const FString jumpSection = FString::FromInt(localDirectionIndex);
	
	FVector moveDirection = m_Owner->GetActorForwardVector();
	
	if (inputInfos.inputVertical != 0 || inputInfos.inputHorizontal != 0) // 방향키입력이 있을경우
	{
		moveDirection =
			controllerForwardVec * inputInfos.inputVertical +
			controllerRightVec * inputInfos.inputHorizontal;

		moveDirection.Normalize();
	}
	
	FVector targetLocation = m_Owner->GetActorLocation() + moveDirection * m_MoveDistance;
	m_Owner->Multicast_SetMotionWarpingTarget(TEXT("Forward"), targetLocation);

	//m_Owner->Multicast_PlayMontage(m_DodgeMontage, 1.0f);
	//m_Owner->Multicast_JumpToMontageSection(m_DodgeMontage, FName(*jumpSection));
	
	const int32 directionIndex = m_Owner->GetDirectionIndexFromKeyInput(inputInfos.inputVertical, inputInfos.inputHorizontal);
	FString jumpSection = FString::FromInt(directionIndex);
	

	if (m_Owner->IsLockOnMode()) // 타겟팅일땐 
	{
		m_Owner->Multicast_PlayMontage(m_DodgeMontage, 1.0f);
		m_Owner->Multicast_JumpToMontageSection(m_DodgeMontage, FName(*jumpSection));
	}
	else
	{
		m_Owner->Multicast_PlayMontage(m_DodgeMontage, 1.0f); // 타겟팅아니면 0번섹션재생 고정.
		
		//jumpSection = FString::FromInt(0);
		//m_Owner->Multicast_JumpToMontageSection(m_DodgeMontage, FName(*jumpSection));
	}
}

bool UDodge_OnGround::CanExecuteSkill() const
{
	return Super::CanExecuteSkill() &&
		!m_OwnerSkillComponent->IsCurSkillState(EMainPlayerSkillStates::Charging_OnGround);
}
