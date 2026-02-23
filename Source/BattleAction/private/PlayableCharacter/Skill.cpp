// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayableCharacter/Skill.h"
#include "PlayableCharacter/PlayableCharacter.h"
#include "MainPlayer/MainPlayerAnim.h"
#include "Component/SkillComponent.h"
#include "Component/StatComponent.h"

USkill::USkill() :
	m_bIsAddedSkillSlots(true),
	m_CoolDownTime(3.0f),
	m_bIsCooldownComplete(true),
	m_StaminaCost(10.0f),
	m_bIsSuperArmor(false),
	m_ThumbnailFillTexture(nullptr),
	m_ThumbnailBackgroundTexture(nullptr)
{
}

void USkill::Execute(const FInputInfos& inputInfos) // �������� ȣ��.
{
	m_bIsCooldownComplete = false;

	m_Owner->GetWorldTimerManager().SetTimer(
		m_TimerHandle,
		[this]()
		{
			m_bIsCooldownComplete = true;
		},
		m_CoolDownTime,
		false);

	m_Owner->SetIsSuperArmor(m_bIsSuperArmor);
	m_Owner->GetStatComponent()->OnDamageStamina(m_StaminaCost * 0.5f);

	if (m_Owner->IsLockOnMode())
	{
		AActor* lockOnTarget = m_Owner->GetCurLockOnTarget();

		if (lockOnTarget != nullptr)
		{
			m_Owner->RotateToTarget(lockOnTarget);
			//m_Owner->Multicast_RotateToTarget(lockOnTarget);
		}
	}
	// else
	// {
	// 	m_Owner->RotateActorToKeyInputDirection(inputInfos.controlYaw, inputInfos.inputVertical, inputInfos.inputHorizontal);
	// }
}

bool USkill::CanExecuteSkill() const
{
	return m_bIsCooldownComplete && m_Owner->HasEnoughStamina(m_StaminaCost);
}

void USkill::SetOwnerInfo(APlayableCharacter* owner)
{
	m_Owner = owner;
	check(m_Owner.IsValid())
	
	m_OwnerAnimInstance = Cast<UAnimInstanceBase>(m_Owner->GetMesh()->GetAnimInstance());
	check(m_OwnerAnimInstance.IsValid());
	
	m_OwnerSkillComponent = m_Owner->GetSkillComponent();
	check(m_OwnerSkillComponent.IsValid());
}



