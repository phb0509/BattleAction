// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase/CharacterBase.h"
#include "CharacterBase/AnimInstanceBase.h"
#include "Utility/Utility.h"
#include "Utility/CustomStructs.h"
#include "Utility/EnumTypes.h"
#include "Component/StatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Component/CrowdControlComponent.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


int32 attackCount = 0; // 로그용
const FName ACharacterBase::KnockbackMontageNames[4] = {"Knockback0", "Knockback1", "Knockback2", "Knockback3"};
const FName ACharacterBase::DeathMontageNames[4] = {"Death0", "Death1", "Death2", "Death3"};


ACharacterBase::ACharacterBase() :
    m_WalkSpeed(200.0f),
    m_RunSpeed(400.0f),
	m_bIsSuperArmor(false),
	m_bIsDead(false)
{
	m_StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	m_StatComponent->OnHPIsZero.AddUObject(this, &ACharacterBase::OnHPIsZero);
	m_StatComponent->OnStaminaIsZero.AddUObject(this, &ACharacterBase::OnStaminaIsZero);

	m_CrowdControlComponent = CreateDefaultSubobject<UCrowdControlComponent>(TEXT("CrowdControlComponent"));
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	attackCount = 0; // 로그용

	check(IsValid(m_StatComponent));
	check(IsValid(m_CrowdControlComponent));

	m_AnimInstanceBase = CastChecked<UAnimInstanceBase>(GetMesh()->GetAnimInstance());

	m_AnimInstanceBase->End_Death.AddUObject(this, &ACharacterBase::OnCalledNotify_End_Death);

	OnTakeDamage.AddUObject(this, &ACharacterBase::PlayOnHitEffect);
}

void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharacterBase, m_bIsSuperArmor);
}

void ACharacterBase::OnDamage(const float finalDamage, const bool bIsCriticalAttack, const FAttackInformation* attackInfo, AActor* instigator, const FVector& causerLocation)
{
	const ECrowdControlType crowdControlType = attackInfo->crowdControlType;
	const float finalCrowdControlTime = m_StatComponent->CalculateFinalCrowdControlTime(attackInfo->crowdControlTime);
	const float _finaldamage = m_StatComponent->CalculateFinalDamage(finalDamage); 

	m_StatComponent->OnDamageHP(_finaldamage);
	m_LastHitDirection = Utility::GetHitDirection(instigator->GetActorLocation(), this); // 占실격뱄옙占쏙옙 占쏙옙占?
	
	const FHitInformation hitInfo =
	{
		attackInfo->attackName,
		instigator,
		this,
		_finaldamage,
		bIsCriticalAttack,
		m_LastHitDirection,
		crowdControlType,
		finalCrowdControlTime,
		attackInfo->knockBackDistance,
		attackInfo->airbornePower,
		attackInfo->staminaDamage
	};
	
	OnTakeDamage.Broadcast(hitInfo); // UI나 이펙트
	
	if (!m_bIsDead && !m_bIsSuperArmor)
	{
		if (crowdControlType != ECrowdControlType::None)
		{
			m_CrowdControlComponent->ApplyCrowdControl(instigator, hitInfo);
		}
	}
	
	// 로그용
	++attackCount;
	const FString log = Tags[0].ToString() + " Takes " + FString::SanitizeFloat(_finaldamage) + " damage from " +
		instigator->Tags[0].ToString() + "::" + attackInfo->attackName.ToString() + "::" + "curHP :: " + FString::FromInt((m_StatComponent->GetCurHP())) + " :: " +
			FString::FromInt(attackCount);
	
	UE_LOG(LogTemp, Warning, TEXT("%s"), *log);
}

void ACharacterBase::OnDamageStamina(const float staminaDamage) const
{
	m_StatComponent->OnDamageStamina(staminaDamage);
}

void ACharacterBase::Server_SetActorLocation_Implementation(const FVector& location)
{
	this->SetActorLocation(location);
}

void ACharacterBase::Server_SetActorRotation_Implementation(const FRotator& rotation)
{
	this->SetActorRotation(rotation);
}

void ACharacterBase::Server_SetMovementMode_Implementation(const EMovementMode movementMode)
{
	GetCharacterMovement()->SetMovementMode(movementMode);
}

void ACharacterBase::Server_SetGravityScale_Implementation(const float gravityScale)
{
	GetCharacterMovement()->GravityScale = gravityScale;
}

void ACharacterBase::OnHPIsZero()
{
	Die();
}

void ACharacterBase::Die()
{
	m_bIsDead = true;
	
	m_CrowdControlComponent->BreakCrowdControlState();
	
	m_StatComponent->StopRecoveryHP();
	m_StatComponent->StopRecoveryStamina();

	m_HitCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	playDeathMontage(m_LastHitDirection);
}

void ACharacterBase::OnCalledNotify_End_Death() 
{
	ExecEvent_EndedDeathMontage();
}

void ACharacterBase::playDeathMontage(const int32 hitDirection)
{
	m_AnimInstanceBase->StopAllMontages(0.0f);
	
	if (m_DeathMontages.Num() >= 1) 
	{
		if (m_DeathMontages.Num() > hitDirection)
		{
			m_AnimInstanceBase->Montage_Play(m_DeathMontages[hitDirection]);
		}
		else
		{
			m_AnimInstanceBase->Montage_Play(m_DeathMontages[0]);
		}
	}
}

void ACharacterBase::PlayOnHitEffect(const FHitInformation& hitInformation)
{
	if (m_HitSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_HitSound, GetActorLocation());
	}
}

void ACharacterBase::RotateToTarget(const AActor* target, const FRotator& rotatorOffset)
{
	// const FVector targetLocation = target->GetActorLocation();
 //    const FVector myLocation = this->GetActorLocation();
	//
 //    const FVector directionToTarget = (targetLocation - myLocation).GetSafeNormal();
 //    const FRotator rotationToTarget = directionToTarget.Rotation();
	//
	// FRotator curRotation = this->GetActorRotation();
	// curRotation.Yaw = rotationToTarget.Yaw;
	// curRotation += rotatorOffset;
	//
	// this->SetActorRotation(curRotation);
	
	if (IsValid(target)) return;
    
	FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(
		GetActorLocation(), 
		target->GetActorLocation()
	);
    
	// Pitch/Roll 제거 (수평 회전만)
	LookAtRot.Pitch = 0.0f;
	LookAtRot.Roll = 0.0f;
    
	SetActorRotation(LookAtRot + rotatorOffset);
}

bool ACharacterBase::IsWithInRange(const AActor* target, const float range) const
{
	if (!IsValid(target))
	{
		return false;
	}
	
	const FVector myLocation = this->GetActorLocation();
	const FVector targetLocation = target->GetActorLocation();
	
	float distance = FVector::DistSquared(myLocation, targetLocation);
	
	return distance <= range * range;
}

FVector ACharacterBase::GetDirectionToTarget(const AActor* target) const
{
	FVector directionToTarget = target->GetActorLocation() - this->GetActorLocation();
	directionToTarget.Z = 0.0f;

	return directionToTarget;
}

bool ACharacterBase::HasEnoughStamina(const float cost) const
{
	return m_StatComponent->HasEnoughStamina(cost);
}

UAnimInstanceBase* ACharacterBase::GetAnimInstanceBase() const
{
	return m_AnimInstanceBase.IsValid() ? m_AnimInstanceBase.Get() : nullptr;
}

UMotionWarpingComponent* ACharacterBase::GetMotionWarpingComponent() const
{
	return m_MotionWarpingComponent.Get();
}

void ACharacterBase::BreakCrowdControlState()
{
	m_CrowdControlComponent->BreakCrowdControlState();
}

bool ACharacterBase::IsGroggy() const
{
	return m_CrowdControlComponent->IsGroggy();
}

float ACharacterBase::GetGroggyTime() const
{
	return m_CrowdControlComponent->GetGroggyTime();
}

void ACharacterBase::RecoveryHP() const
{
	m_StatComponent->RecoveryHP();
}

void ACharacterBase::RecoveryStamina() const
{
	m_StatComponent->RecoveryStamina();
}

void ACharacterBase::SetInvincible(const bool bIsInvincible)
{
	if (IsValid(m_HitCollider))
	{
		if (bIsInvincible)
		{
			m_HitCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			m_HitCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
	}
}

UShapeComponent* ACharacterBase::GetCollider(const FName& colliderName) const
{
	return m_Colliders.Contains(colliderName) ? m_Colliders[colliderName].Get() : nullptr;
}

bool ACharacterBase::IsCrowdControlState() const
{
	return m_CrowdControlComponent->IsCrowdControlState();
}

void ACharacterBase::SetAllComponentsTickEnabled(bool bEnabled)
{
	TInlineComponentArray<UActorComponent*> Components;
	GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component)
		{
			Component->SetComponentTickEnabled(bEnabled);
		}
	}
}

void ACharacterBase::SetActorLocationReplicated(const FVector& location)
{
	this->SetActorLocation(location);
	this->Server_SetActorLocation(location);
}

void ACharacterBase::SetActorRotationReplicated(const FRotator& rotation)
{
	this->SetActorRotation(rotation);
	this->Server_SetActorRotation(rotation);
}

void ACharacterBase::SetMovementModeReplicated(const EMovementMode movementMode)
{
	this->GetCharacterMovement()->SetMovementMode(movementMode);
	this->Server_SetMovementMode(movementMode);
}

void ACharacterBase::SetGravityScaleReplicated(const float gravityScale)
{
	GetCharacterMovement()->GravityScale = gravityScale;
	this->Server_SetGravityScale(gravityScale);
}

void ACharacterBase::Multicast_PlayMontage_Implementation(UAnimMontage* montage, float playRate)
{
	if (IsValid(montage) && m_AnimInstanceBase.IsValid())
	{
		m_AnimInstanceBase->Montage_Play(montage, playRate);
	}
}

void ACharacterBase::Multicast_JumpToMontageSection_Implementation(UAnimMontage* montage, const FName& sectionName)
{
	if (IsValid(montage) && m_AnimInstanceBase.IsValid())
	{
		m_AnimInstanceBase->Montage_JumpToSection(sectionName, montage);
	}
}

void ACharacterBase::Multicast_StopAllMontages_Implementation(const float blendOut)
{
	if (m_AnimInstanceBase.IsValid())
	{
		m_AnimInstanceBase->StopAllMontages(blendOut);
	}
}

void ACharacterBase::Multicast_SetMotionWarpingTarget_Implementation(const FName& warpTargetName, const FVector& targetLocation)
{
	if (IsValid(m_MotionWarpingComponent))
	{
		m_MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(warpTargetName, targetLocation);
	}
}

void ACharacterBase::Multicast_RotateToTarget_Implementation(AActor* target, const FRotator& rotatorOffset)
{
	RotateToTarget(target, rotatorOffset);
}




