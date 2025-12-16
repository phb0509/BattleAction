
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
				OnEndedGroggy.Broadcast(); // ���׹̳� 100% + ü�� �� ���׹̳� �ڰ�ȸ�� ����.
			},
		m_CrowdControlSetting.groggyTime,
		false);
}

void UCrowdControlComponent::TakeAttack_Knockback(AActor* instigator, const FHitInformation& attackInfo)
{
	if (m_CurCrowdControlState == ECrowdControlType::Down) // �ٿ� ����
	{
		CallTimer_CheckOnGround();
	}
	else if (m_CurCrowdControlState == ECrowdControlType::Airborne) // ��� ����
	{
		playCrowdControlMontage(ECrowdControlType::Airborne, attackInfo.hitDirection);

		DisableMovementComponentForDuration(0.2f);
		CallTimer_CheckOnGround();
	} 
	else // �����̻� X or �˹� or �׷α�
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

void UCrowdControlComponent::OnCalledTimer_KnockbackOnStanding_End() // �˹�CC�ð� ���� �� ȣ��
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
	if (m_CurCrowdControlState == ECrowdControlType::Airborne) // ��� ����
	{
		playCrowdControlMontage(ECrowdControlType::Airborne, attackInfo.hitDirection);
		DisableMovementComponentForDuration(0.2f); 
	}
	else // �����̻� X or �ٿ� or �˹� or �׷α�
	{
		this->SetCrowdControlState(ECrowdControlType::Down);
		
		playCrowdControlMontage(ECrowdControlType::Down, attackInfo.hitDirection);
		
		UAnimMontage* downMontage = m_CrowdControlSetting.crowdControlMontages[ECrowdControlType::Down].montages[0];
		const float downTime = m_OwnerAnimInstance->GetMontagePlayTime(downMontage) + 0.2f; // CC�ð��� ��Ÿ������ð����� ª����츦 ����� ������
		
		GetOwner()->GetWorldTimerManager().SetTimer(
			m_CrowdControlTimerHandle,
			this,
			&UCrowdControlComponent::GetUp,
			attackInfo.crowdControlTime > downTime ? attackInfo.crowdControlTime : downTime,
			false);
	}
}

void UCrowdControlComponent::CallTimer_CheckOnGround()
{
	m_Owner->GetWorldTimerManager().SetTimer(m_CrowdControlTimerHandle,
		this,
			&UCrowdControlComponent::CheckOnGround,	
			GetWorld()->DeltaTimeSeconds,
			true,
			-1);
}

void UCrowdControlComponent::CheckOnGround() // OnGround���� ƽ���� ȣ��Ǿ����� �Լ�.
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

	if (m_CurCrowdControlState == ECrowdControlType::Down) // �ٿ� ����
	{
		airbornePower.Z /= 2; // �ٿ���¿����� ����� �������� ���δ�.
		
		playCrowdControlMontage(ECrowdControlType::Down, attackInfo.hitDirection);
	}
	else // �˹� or ��� or �׷α�
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
	if (!m_CrowdControlSetting.crowdControlMontages.Contains(crowdControlType))
	{
		return;
	}
	
	auto montages = m_CrowdControlSetting.crowdControlMontages[crowdControlType].montages;

	if (montages.Num() >= 1) // �ּ� 1���̻�������
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
	ClearGroggyTimerHandle(); // �̰� ���� �ȵǼ� ��Ŀ������ �� isgroggy�� true.
	
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



