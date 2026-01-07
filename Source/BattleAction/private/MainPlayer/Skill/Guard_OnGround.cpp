// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayer/Skill/Guard_OnGround.h"
#include "MainPlayer/MainPlayer.h"
#include "MainPlayer/MainPlayerAnim.h"
#include "Component/MainPlayerSkillComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/EnumTypes.h"


UGuard_OnGround::UGuard_OnGround() :
	m_GuardMontage(nullptr),
	m_ParryingTime(0.5f),
	m_ParryingShake(nullptr),
	m_ParryingSound(nullptr),
	m_ParryingHitParticle(nullptr)
{
}

void UGuard_OnGround::Initialize()
{
	Super::Initialize();
	
	check(m_GuardMontage != nullptr);
	check(m_ParryingShake != nullptr);
	check(m_ParryingSound != nullptr);
	check(m_ParryingHitParticle != nullptr);
}

void UGuard_OnGround::Execute()
{
	Super::Execute();

	AMainPlayer* owner = CastChecked<AMainPlayer>(m_Owner);
	
	m_OwnerSkillComponent->SetSkillState(EMainPlayerSkillStates::Parry_OnGround);
	m_OwnerAnimInstance->Montage_Play(m_GuardMontage, 1.0f);
	
	owner->GetCollider(TEXT("ShieldForGuardCollider"))->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//owner->SetIsParrying(true);
	FDelegateHandle handle = owner->OnDamageOverride.AddUObject(this, &UGuard_OnGround::onParrySuccess);

	owner->GetWorldTimerManager().SetTimer(
	   m_ParryingTimer,
	   [owner, handle]()
	   {
		   owner->OnDamageOverride.Remove(handle);
		   owner->GetCollider(TEXT("ShieldForGuardCollider"))->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	   },
	   m_ParryingTime,
	   false
	);
	
}

void UGuard_OnGround::onParrySuccess(AActor* instigator)
{
	ACharacterBase* attacker = CastChecked<ACharacterBase>(instigator);
	attacker->OnDamageStamina(1000.0f); // 거의 강제 스턴유도
	
	m_Owner->GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(m_ParryingShake);
	UGameplayStatics::PlaySoundAtLocation(m_Owner.Get(), m_ParryingSound, m_Owner->GetActorLocation());

	if (m_ParryingHitParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			m_ParryingHitParticle,
			m_Owner->GetActorLocation(),
			FRotator::ZeroRotator,
			true // bAutoDestroy
		);
	}
	
}

bool UGuard_OnGround::CanExecuteSkill() const
{
	if (m_OwnerSkillComponent->IsCurSkillState(EMainPlayerSkillStates::None))
	{
		return Super::CanExecuteSkill() && !m_Owner->IsCrowdControlState();
	}
	
	bool skillRequirements =
		m_OwnerSkillComponent->IsCurSkillState(EMainPlayerSkillStates::NormalAttack_OnGround) ||
		m_OwnerSkillComponent->IsCurSkillState(EMainPlayerSkillStates::NormalStrikeAttack_OnGround);
	
	return Super::CanExecuteSkill() && skillRequirements;
}
