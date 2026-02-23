// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/SkillComponent.h"
#include "CharacterBase/AnimInstanceBase.h"
#include "PlayableCharacter/PlayableCharacter.h"
#include "PlayableCharacter/Skill.h"
#include "SubSystems/UIManager.h"

USkillComponent::USkillComponent() :
	m_CurSkillState(0),
	m_GravityScaleInAir(0.00001f)
{
	PrimaryComponentTick.bCanEverTick = false; 
	bWantsInitializeComponent = false;
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Owner = CastChecked<APlayableCharacter>(GetOwner());
	m_OwnerAnimInstance = CastChecked<UAnimInstanceBase>(m_Owner->GetMesh()->GetAnimInstance());

	loadSkills();
	
	if (m_Owner->IsLocallyControlled())
	{
		GetWorld()->GetGameInstance()->GetSubsystem<UUIManager>()->CreateSkillSlots(this, m_Owner.Get());
	}
}

void USkillComponent::ExecuteSkill(const FName& inputMappingContextName, const FName& skillName)
{
	if (HasSkill(inputMappingContextName, skillName))
	{
		USkill* skill = m_SkillList[inputMappingContextName].skillList[skillName];

		if (skill->CanExecuteSkill())  // 클라에서 먼저 체크
		{
			if (skill->GetIsAddedSkillSlots())
			{
				skill->OnExecute.Broadcast(); // 스킬아이콘 쿨다운 업데이트.
			}

			//Server_ExecuteSkill(inputMappingContextName, skillName, BuildInputInfos());

			FInputInfos inputInfos = BuildInputInfos();

			// 서버 RPC보다 먼저 클라이언트에서 즉시 회전 적용.
			// Execute()는 서버에서만 호출되므로, Multicast 몽타주/워프 도착 전에 클라 회전을 맞춰줘야 함.
			if (m_Owner->IsLockOnMode())
			{
				AActor* lockOnTarget = m_Owner->GetCurLockOnTarget();
				if (lockOnTarget != nullptr)
				{
					m_Owner->RotateToTarget(lockOnTarget);
				}
			}
			// else
			// {
			// 	m_Owner->RotateActorToKeyInputDirection(inputInfos.controlYaw, inputInfos.inputVertical, inputInfos.inputHorizontal);
			// }

			Server_ExecuteSkill(inputMappingContextName, skillName, inputInfos);
		}
	}
}

void USkillComponent::Server_ExecuteSkill_Implementation(const FName& inputMappingContextName, const FName& skillName, const FInputInfos& inputInfos)
{
	if (HasSkill(inputMappingContextName, skillName))  // nullptr 방지
	{
		m_CurSkill = m_SkillList[inputMappingContextName].skillList[skillName];

		if (m_CurSkill->CanExecuteSkill())  // 서버에서 검증
		{
			m_CurSkill->Execute(inputInfos);
		}
	}
}

void USkillComponent::ApplyInAirSkillGravity()
{
	m_Owner->SetGravityScaleReplicated(m_GravityScaleInAir);
}

void USkillComponent::InitGravityScaleAfterAttack()
{
	m_Owner->SetGravityScaleReplicated(1.0f);
}

FInputInfos USkillComponent::BuildInputInfos() const
{
	FInputInfos inputInfos;
	inputInfos.inputVertical = m_Owner->GetCurInputVertical();
	inputInfos.inputHorizontal = m_Owner->GetCurInputHorizontal();
	inputInfos.controlYaw = m_Owner->GetControlRotation().Yaw;

	return inputInfos;
}

bool USkillComponent::HasSkill(const FName& inputMappingContextName, const FName& skillName) const
{
	return m_SkillList.Contains(inputMappingContextName) &&
			m_SkillList[inputMappingContextName].skillList.Contains(skillName) &&
			m_SkillList[inputMappingContextName].skillList[skillName] != nullptr;
}

void USkillComponent::loadSkills()
{
	for (const auto iter : m_SkillClassList)
	{
		const FName inputMappingContextName = iter.Key;

		FSkillList skillList;
		m_SkillList.Add(inputMappingContextName, skillList);
		
		for (const TSubclassOf<USkill> skillClass : iter.Value.skillClassList)
		{
			USkill* skill = Cast<USkill>(NewObject<UObject>(GetTransientPackage(), skillClass));
			skill->SetOwnerInfo(m_Owner.Get());
			skill->Initialize();
			
			const FName skillName = skill->GetName();
			m_SkillList[inputMappingContextName].skillList.Add(skillName, skill);
		}
	}
}


