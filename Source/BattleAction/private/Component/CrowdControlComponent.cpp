
#include "Component/CrowdControlComponent.h"
#include "CharacterBase/AnimInstanceBase.h"
#include <GameFramework/CharacterMovementComponent.h>
#include "BehaviorTree/BlackboardComponent.h"
#include "CharacterBase/AIControllerBase.h"
#include "PlayableCharacter/PlayableCharacter.h"
#include "Utility/EnumTypes.h"
#include "Utility/CustomStructs.h"


UCrowdControlComponent::UCrowdControlComponent() :
	m_Owner(nullptr),
	m_CrowdControlSetting({}),
	m_CharacterMovementComponent(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
	
	const UEnum* enumPtr = StaticEnum<ECrowdControlType>();
	
	if (enumPtr != nullptr)
	{
		for (int i = 0; i < enumPtr->NumEnums(); ++i)
		{
			ECrowdControlType state = static_cast<ECrowdControlType>(enumPtr->GetValueByIndex(i));

			if (state == ECrowdControlType::None)
			{
				continue;
			}
			
			FOnCrowdControl_Start_Delegate startDelegate;
			m_CrowdControlStartDelegates.Add(state,startDelegate);
		}
	}
}

void UCrowdControlComponent::BeginPlay()
{
	Super::BeginPlay();

	m_Owner = CastChecked<ACharacterBase>(GetOwner());
	m_OwnerAnimInstance = CastChecked<UAnimInstanceBase>(m_Owner->GetMesh()->GetAnimInstance());

	if (!m_Owner->IsA(APlayableCharacter::StaticClass()))
	{
		m_OwnerAIController = CastChecked<AAIController>(GetOwner()->GetInstigatorController());
	}
	
	m_CrowdControlStartDelegates[ECrowdControlType::Knockback].AddUObject(this, &UCrowdControlComponent::TakeAttack_Knockback);
	m_CrowdControlStartDelegates[ECrowdControlType::Down].AddUObject(this, &UCrowdControlComponent::TakeAttack_Down);
    m_CrowdControlStartDelegates[ECrowdControlType::Airborne].AddUObject(this, &UCrowdControlComponent::TakeAttack_Airborne);
}

void UCrowdControlComponent::ApplyCrowdControl(AActor* instigator, const FHitInformation& attackInfo)
{
	if (m_OwnerAIController != nullptr)
	{
		m_OwnerAIController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsCrowdControlState"), true);
	}
	
	m_CrowdControlStartDelegates[attackInfo.crowdControlType].Broadcast(instigator, attackInfo);
	
	if (attackInfo.knockBackDistance > 0.0f)
	{
		applyKnockback(attackInfo);
	}
}

void UCrowdControlComponent::applyKnockback(const FHitInformation& hitInfo)
{
	AActor* instigator = hitInfo.instigator.Get();
	
	FVector dirToInstigator = instigator->GetActorLocation() - m_Owner->GetActorLocation(); 
	dirToInstigator.Normalize();
	dirToInstigator *= -1 * hitInfo.knockBackDistance;
	dirToInstigator.Z = 0.0f;
	
	hitInfo.hitActor->SetActorLocation(m_Owner->GetActorLocation() + dirToInstigator, false);
}

void UCrowdControlComponent::OnGroggy()
{
	if (IsGroggy()) 
	{
		return;
	}
	
	if (m_CurCrowdControlState == ECrowdControlType::None || m_CurCrowdControlState == ECrowdControlType::Knockback)
	{
		ClearCrowdControlTimerHandle();
		playCrowdControlMontage(ECrowdControlType::Groggy, 0);

		this->SetCrowdControlState(ECrowdControlType::Groggy);
	}
	
	m_Owner->GetWorldTimerManager().SetTimer
		(
			m_GroggyTimerHandle, 
			[this]()
			{
				if (!m_Owner->IsDead() && m_CurCrowdControlState == ECrowdControlType::Groggy)
				{
					m_OwnerAnimInstance->StopAllMontages(0.0f);
					
					this->SetCrowdControlState(ECrowdControlType::None);
				}

				ClearGroggyTimerHandle();
				OnEndedGroggy.Broadcast(); // 스테미너 100%회복 및, 다시 체력재생 시작.
			},
		m_CrowdControlSetting.groggyTime,
		false);
}

void UCrowdControlComponent::TakeAttack_Knockback(AActor* instigator, const FHitInformation& attackInfo)
{
	if (m_CurCrowdControlState == ECrowdControlType::Down) // 다운중에 넉백공격 받을경우,
	{
		CallTimer_CheckOnGround();
	}
	else if (m_CurCrowdControlState == ECrowdControlType::Airborne) // 에어본중에 넉백공격 받을경우,
	{
		playCrowdControlMontage(ECrowdControlType::Airborne, attackInfo.hitDirection);

		DisableMovementComponentForDuration(0.2f);
		CallTimer_CheckOnGround();
	} 
	else // CC상태 아니거나, 넉백중이거나
	{
		this->SetCrowdControlState(ECrowdControlType::Knockback);
		
		playCrowdControlMontage(ECrowdControlType::Knockback, attackInfo.hitDirection);

		GetOwner()->GetWorldTimerManager().SetTimer(
			m_CrowdControlTimerHandle,
			this,
			&UCrowdControlComponent::OnCalledTimer_KnockbackOnStanding_End,
			attackInfo.crowdControlTime, false);
	}

}

void UCrowdControlComponent::OnCalledTimer_KnockbackOnStanding_End() // 넉백끝난이후 호출
{
	if (m_Owner->IsDead())
	{
		BreakCrowdControlState();
		
		return;
	}
	
	if (m_CurCrowdControlState == ECrowdControlType::Knockback)
	{
		if (IsGroggy())
		{
			this->SetCrowdControlState(ECrowdControlType::Groggy);
			
			playCrowdControlMontage(ECrowdControlType::Groggy, 0);
		}
		else
		{
			this->SetCrowdControlState(ECrowdControlType::None);
			
			m_OwnerAnimInstance->StopAllMontages(0.0f);
		}
	}
}

void UCrowdControlComponent::TakeAttack_Down(AActor* instigator, const FHitInformation& attackInfo)
{
	if (m_CurCrowdControlState == ECrowdControlType::Airborne) // 에어본중에 다운공격 받을경우,
	{
		playCrowdControlMontage(ECrowdControlType::Airborne, attackInfo.hitDirection);
		DisableMovementComponentForDuration(0.2f); 
	}
	else // 이외의 경우에 다운공격 받을 경우, 다운몽타주 재생하며 다운 유지.
	{
		this->SetCrowdControlState(ECrowdControlType::Down);
		
		playCrowdControlMontage(ECrowdControlType::Down, attackInfo.hitDirection);
		
		UAnimMontage* downMontage = m_CrowdControlSetting.crowdControlMontages[ECrowdControlType::Down].montages[0];
		const float downTime = m_OwnerAnimInstance->GetMontagePlayTime(downMontage) + 0.2f; // 다운시간 너무 짧은경우를 대비하기 위한 약간의 오프셋값.
		
		GetOwner()->GetWorldTimerManager().SetTimer(
			m_CrowdControlTimerHandle,
			this,
			&UCrowdControlComponent::GetUp,
			attackInfo.crowdControlTime > downTime ? attackInfo.crowdControlTime : downTime,
			false);
	}
}

void UCrowdControlComponent::CallTimer_CheckOnGround() // 공중에 띄워질 경우(에어본 공격), 땅에 떨어질 때 까지 체크하는 함수 호출할 타이머함수.
{
	m_Owner->GetWorldTimerManager().SetTimer(m_CrowdControlTimerHandle,
		this,
			&UCrowdControlComponent::CheckOnGround,	
			GetWorld()->DeltaTimeSeconds,
			true,
			-1);
}

void UCrowdControlComponent::CheckOnGround() 
{
	if (m_Owner->IsDead())
	{
		BreakCrowdControlState();
		
		return;
	}

	if (getCharacterMovementComponent()->IsMovingOnGround()) 
	{
		m_Owner->GetWorldTimerManager().ClearTimer(m_CrowdControlTimerHandle);

		this->SetCrowdControlState(ECrowdControlType::Down);
		
		playCrowdControlMontage(ECrowdControlType::Down, 0);
		
		UAnimMontage* downMontage = m_CrowdControlSetting.crowdControlMontages[ECrowdControlType::Down].montages[0];
		const float downPlayTime = m_OwnerAnimInstance->GetMontagePlayTime(downMontage) + 0.2f;

		m_Owner->GetWorldTimerManager().SetTimer(
			m_CrowdControlTimerHandle,
			this,
			&UCrowdControlComponent::GetUp,
			m_LastCrowdControlTime > downPlayTime ? m_LastCrowdControlTime : downPlayTime,
			false);
	}
}

void UCrowdControlComponent::GetUp()
{
	UAnimMontage* getUpMontage = m_CrowdControlSetting.getUpMontage;

	if (getUpMontage != nullptr)
	{
		m_OwnerAnimInstance->Montage_Play(getUpMontage);
	
		const float getupPlayTime = m_OwnerAnimInstance->GetMontagePlayTime(getUpMontage);
	
		m_Owner->GetWorldTimerManager().SetTimer(
			m_CrowdControlTimerHandle,
			[this]()
			{
				if (IsGroggy())
				{
					this->SetCrowdControlState(ECrowdControlType::Groggy);
					
					playCrowdControlMontage(ECrowdControlType::Groggy, 0);
				}
				else
				{
					this->SetCrowdControlState(ECrowdControlType::None);
				}
			},
			getupPlayTime,
			false);
	}
}

void UCrowdControlComponent::TakeAttack_Airborne(AActor* instigator, const FHitInformation& attackInfo)
{
	FVector airbornePower = {0.0f, 0.0f, attackInfo.airbornePower};

	if (m_CurCrowdControlState == ECrowdControlType::Down) // 다운상태일 때, 에어본공격 받을경우
	{
		airbornePower.Z /= 2; // 다운상태인 경우에는 약간만 에어본 되게 보정
		
		playCrowdControlMontage(ECrowdControlType::Down, attackInfo.hitDirection);
	}
	else // 이외의 상태에선 에어본몽타주 재생 및 상태유지.
	{
		this->SetCrowdControlState(ECrowdControlType::Airborne);
		
		playCrowdControlMontage(ECrowdControlType::Airborne, attackInfo.hitDirection);
	}

	const FVector launchVelocity = airbornePower;
	m_Owner->LaunchCharacter(launchVelocity, true, true);

	CallTimer_CheckOnGround();
}

void UCrowdControlComponent::DisableMovementComponentForDuration(float duration)
{
	getCharacterMovementComponent()->Deactivate();

	FTimerHandle activateTimer;
	m_Owner->GetWorldTimerManager().SetTimer(activateTimer,
		[this]()
		{
			getCharacterMovementComponent()->Activate();
		},
			duration,
			false);
}

UCharacterMovementComponent* UCrowdControlComponent::getCharacterMovementComponent()
{
	if (m_CharacterMovementComponent == nullptr)
	{
		m_CharacterMovementComponent = m_Owner->GetCharacterMovement();
	}
	
	return m_CharacterMovementComponent.IsValid() ? m_CharacterMovementComponent.Get() : nullptr;
}

void UCrowdControlComponent::playCrowdControlMontage(const ECrowdControlType crowdControlType, const int32 hitDirection)
{
	if (!m_CrowdControlSetting.crowdControlMontages.Contains(crowdControlType)) // CC상태자체가 없는경우,
	{
		return;
	}
	
	auto montages = m_CrowdControlSetting.crowdControlMontages[crowdControlType].montages;

	if (montages.Num() >= 1) // CC상태는 설정 및 몽타주 있는 경우,
	{
		if (montages.Num() > hitDirection)
		{
			m_OwnerAnimInstance->Montage_Play(montages[hitDirection]);
		}
		else
		{
			m_OwnerAnimInstance->Montage_Play(montages[0]);
		}
	}
}

void UCrowdControlComponent::endedCrowdControl()
{
	this->SetCrowdControlState(ECrowdControlType::None);
}

void UCrowdControlComponent::BreakCrowdControlState()
{
	ClearCrowdControlTimerHandle();
	ClearGroggyTimerHandle(); 
	
	this->SetCrowdControlState(ECrowdControlType::None);
}

bool UCrowdControlComponent::IsCrowdControlState() const
{
	return m_CurCrowdControlState != ECrowdControlType::None || IsGroggy();
}

bool UCrowdControlComponent::IsGroggy() const
{
	return m_Owner->GetWorldTimerManager().IsTimerActive(m_GroggyTimerHandle);
}

void UCrowdControlComponent::SetCrowdControlState(ECrowdControlType crowdControlType)
{
	m_CurCrowdControlState = crowdControlType;

	if (m_OwnerAIController.IsValid())
	{
		m_OwnerAIController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsCrowdControlState"), crowdControlType != ECrowdControlType::None);
	}
}

void UCrowdControlComponent::ClearCrowdControlTimerHandle()
{
	m_Owner->GetWorldTimerManager().ClearTimer(m_CrowdControlTimerHandle);
}

void UCrowdControlComponent::ClearGroggyTimerHandle()
{
	m_Owner->GetWorldTimerManager().ClearTimer(m_GroggyTimerHandle);
}



