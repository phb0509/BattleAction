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

void UDodge_OnGround::Execute()
{
	Super::Execute();

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
	m_OwnerAnimInstance->StopAllMontages(0.0f);
	
	// ?ㅼ엯?μ뿉 ?곕Ⅸ ?붾뱶諛⑺뼢踰≫꽣 援ы븯湲?
	const FVector worldKeyInputDirection = m_Owner->GetWorldKeyInputDirection(m_Owner->GetDirectionIndexFromKeyInput());

	// 罹먮┃?곌린以?쇰줈 ?대뵒諛⑺뼢?몄? 援ы븯湲?
	const int32 localDirectionIndex = m_Owner->GetLocalDirectionIndex(worldKeyInputDirection);
	const FString jumpSection = FString::FromInt(localDirectionIndex);
	
	m_OwnerAnimInstance->Montage_Play(m_DodgeMontage,1.0f);
	m_OwnerAnimInstance->Montage_JumpToSection(*jumpSection, m_DodgeMontage);

	FVector targetVerticalVector;
	FVector targetHorizontalVector;
	
	if (!m_Owner->IsMoveKeyPressed()) // ?ㅼ엯???놁쓣 ??
	{
		targetVerticalVector = m_Owner->GetForwardVectorFromControllerYaw() * m_MoveDistance;
		targetHorizontalVector = {0.0f, 0.0f, 0.0f};
	}
	else
	{
		targetVerticalVector = m_Owner->GetForwardVectorFromControllerYaw() * m_MoveDistance * m_Owner
			->GetCurInputVertical();
		targetHorizontalVector = m_Owner->GetRightVectorFromControllerYaw() * m_MoveDistance * m_Owner
			->GetCurInputHorizontal();
	}

	m_Owner->GetMotionWarpingComponent()->AddOrUpdateWarpTargetFromLocation(
		TEXT("Forward"), m_Owner->GetActorLocation() + targetVerticalVector + targetHorizontalVector);
	
}

bool UDodge_OnGround::CanExecuteSkill() const
{
	return Super::CanExecuteSkill() &&
		!m_OwnerSkillComponent->IsCurSkillState(EMainPlayerSkillStates::Charging_OnGround);
}
