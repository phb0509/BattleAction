//Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Monster.h"
#include "SignificanceManager.h"
#include "CharacterBase/AIControllerBase.h"
#include "SubSystems/UIManager.h"
#include "Component/StatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Component/CrowdControlComponent.h"
#include "Components/TimelineComponent.h"
#include "Utility/CustomStructs.h"
#include "GameFramework/Character.h"

const FName AMonster::HomePosKey(TEXT("HomePos"));
const FName AMonster::PatrolPosKey(TEXT("PatrolPos"));
const FName AMonster::EnemyKey(TEXT("Enemy"));
const FName AMonster::FSMStateKey(TEXT("FSMState"));
const FName AMonster::IsCrowdControlState(TEXT("IsCrowdControlState"));


AMonster::AMonster() :
	m_CurFSMState(0),
	m_DeathCurveFloat(nullptr),
	m_DeathDissolveCurveFloat(nullptr)
{
}

void AMonster::BeginPlay()
{
	Super::BeginPlay();

	m_AIControllerBase = CastChecked<AAIControllerBase>(GetController());

	check(IsValid(m_CrowdControlComponent));
	m_CrowdControlComponent->OnEndedGroggy.AddUObject(this, &AMonster::EndedGroggy);
	
	setTimeline();
	optimize();
}

void AMonster::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	m_DeathTimeline.TickTimeline(DeltaSeconds);
	m_DeathDissolveTimeline.TickTimeline(DeltaSeconds);
}

void AMonster::OnDamage(const float damage, const bool bIsCriticalAttack, const FAttackInformation* AttackInformation,
	AActor* instigator, const FVector& causerLocation)
{
	Super::OnDamage(damage, bIsCriticalAttack, AttackInformation, instigator, causerLocation);
	
	m_AIControllerBase->GetBlackboardComponent()->SetValueAsObject(AMonster::EnemyKey, instigator);
	
	if (!IsDead() && !m_CrowdControlComponent->IsGroggy())
	{
		check(m_StatComponent != nullptr);
		m_StatComponent->OnDamageStamina(AttackInformation->staminaDamage);
	}
}

void AMonster::OnStaminaIsZero()
{
	Super::OnStaminaIsZero();
	
	this->OnGroggy();
}

void AMonster::OnGroggy()
{
	m_StatComponent->StopRecoveryHP();
	m_StatComponent->StopRecoveryStamina();
	
	m_CrowdControlComponent->OnGroggy();
}

void AMonster::EndedGroggy()
{
	m_StatComponent->SetStaminaPercent(100.0f); 
	m_StatComponent->RecoveryHP(); 
	m_StatComponent->RecoveryStamina(); 
}

void AMonster::Die()
{
	Super::Die();

	this->SetIsDead(true);
	
	m_AIControllerBase->GetBlackboardComponent()->SetValueAsObject(AMonster::EnemyKey, nullptr);
	m_AIControllerBase->StopBehaviorTree();
}

void AMonster::ExecEvent_EndedDeathMontage()
{
	Super::ExecEvent_EndedDeathMontage();

	m_DeathTimeline.Play();
}

void AMonster::OnCalledTimelineEvent_Loop_AfterDeath(float curveValue)
{
	this->GetMesh()->SetScalarParameterValueOnMaterials(TEXT("DiffuseRatioOnDeath"), curveValue * 2);
}

void AMonster::OnCalledTimelineEvent_End_AfterDeath()
{
	// 디졸브효과 시작.
	m_DeathDissolveTimeline.Play();
}

void AMonster::OnCalledTimelineEvent_Loop_DeathDissolve(float curveValue)
{
	this->GetMesh()->SetScalarParameterValueOnMaterials(TEXT("DissolveAmount"), curveValue);
}

void AMonster::OnCalledTimelineEvent_End_DeathDissolve()
{
	m_DeathDissolveTimeline.SetNewTime(0.0f);
	m_DeathTimeline.SetNewTime(0.0f); // 초기화

	this->Deactivate();
}

void AMonster::Initialize()
{
	// HPBar 위젯 생성 및 부착.
	UUIManager* uiManager = GetWorld()->GetGameInstance()->GetSubsystem<UUIManager>();
	check(uiManager != nullptr);
	
	uiManager->CreateMonsterHPBar(this);
}

void AMonster::Activate()
{
	this->SetIsDead(false);
	
	m_CrowdControlComponent->SetCrowdControlState(ECrowdControlType::None);
	
	m_StatComponent->InitHP();
	m_StatComponent->InitStamina();
	
	m_AIControllerBase->OnPossess(this);
	m_AIControllerBase->StartBehaviorTree();

	GetMesh()->SetScalarParameterValueOnMaterials(TEXT("DiffuseRatio"), 0.0f);

	// 충돌체들 활성화
	TWeakObjectPtr<UShapeComponent> hitCollider = m_Colliders[TEXT("HitCollider")];
	check(hitCollider.IsValid());
	
	hitCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	this->SetActorTickEnabled(true);
	this->SetActorHiddenInGame(false);

	OnTakeDamage.AddUObject(this, &AMonster::PlayOnHitEffect);
	
	// UI
	UUIManager* uiManager = GetWorld()->GetGameInstance()->GetSubsystem<UUIManager>();
	check(uiManager != nullptr);
	
	OnTakeDamage.AddUObject(uiManager, &UUIManager::RenderDamageToScreen);
}

void AMonster::Deactivate() // 액터풀에서 첫생성하거나 사망 후 회수되기 직전에 호출.
{
	this->SetIsDead(true);

	m_AIControllerBase->StopBehaviorTree();
	m_AIControllerBase->OnUnPossess();

	OnTakeDamage.Clear();

	GetMesh()->GetAnimInstance()->StopAllMontages(0.0f);


	// 충돌체 비활성화.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	for (const auto collider : m_Colliders)
	{
		if (collider.Value.IsValid())
		{
			collider.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	deoptimize();

	// this->Pause();
	// GetMesh()->SetHiddenInGame(true);
	// this->SetTickableWhenPaused(true);
	this->SetActorTickEnabled(false);
	this->SetActorHiddenInGame(true);
}

bool AMonster::IsActive()
{
	return !IsHidden();
}

void AMonster::setTimeline()
{
	check(m_DeathCurveFloat != nullptr);
	check(m_DeathDissolveCurveFloat != nullptr);
	
	if (m_DeathCurveFloat != nullptr)
	{
		m_DeathTimeline.SetLooping(false);

		FOnTimelineFloat afterDeathTimeline_Loop;
		afterDeathTimeline_Loop.BindDynamic(this, &AMonster::OnCalledTimelineEvent_Loop_AfterDeath);
		m_DeathTimeline.AddInterpFloat(m_DeathCurveFloat, afterDeathTimeline_Loop);

		FOnTimelineEvent afterDeathTimeline_End;
		afterDeathTimeline_End.BindDynamic(this, &AMonster::OnCalledTimelineEvent_End_AfterDeath);
		m_DeathTimeline.SetTimelineFinishedFunc(afterDeathTimeline_End);
	}

	if (m_DeathDissolveCurveFloat != nullptr)
	{
		m_DeathDissolveTimeline.SetLooping(false);

		FOnTimelineFloat dissolveDeathTimeline_Loop;
		dissolveDeathTimeline_Loop.BindDynamic(this, &AMonster::OnCalledTimelineEvent_Loop_DeathDissolve);
		m_DeathDissolveTimeline.AddInterpFloat(m_DeathDissolveCurveFloat, dissolveDeathTimeline_Loop);

		FOnTimelineEvent dissolvTimeline_End;
		dissolvTimeline_End.BindDynamic(this, &AMonster::OnCalledTimelineEvent_End_DeathDissolve);
		m_DeathDissolveTimeline.SetTimelineFinishedFunc(dissolvTimeline_End);
	}
}

void AMonster::optimize()
{
	// 애니메이션 최적화 설정
	if (USkeletalMeshComponent* mesh = GetMesh())
	{
		// 화면 밖에서는 애니메이션 업데이트 안 함. 
		mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	
		// URO (Update Rate Optimization) 활성화. 
		mesh->bEnableUpdateRateOptimizations = true;
	}

	//Significance Manager 최적화 (토글 가능)
	if (m_bUseSignificanceManager)
	{
		if (USignificanceManager* significanceManager = USignificanceManager::Get<USignificanceManager>(GetWorld()))
		{
			// 자동으로 플레이어와의 거리 계산, 화면 내 여부 등을 계산
			significanceManager->RegisterObject(
				this,
				TEXT("Monster"),
				[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& ViewPoint) -> float
				{
					// 거리 계산 (가까울수록 높은 값)
					const float distance = FVector::Dist(GetActorLocation(), ViewPoint.GetLocation());
					const float maxDistance = 300.0f; // 최대 고려 거리

					// 0.0(멀리) ~ 1.0(가까이)
					return FMath::Clamp(1.0f - (distance / maxDistance), 0.0f, 1.0f);
				},
				USignificanceManager::EPostSignificanceType::Sequential,
				[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float NewSignificance, bool bFinal)
				{
					// Significance 값이 변경될 때 호출
					UpdateOptimizationLevel(NewSignificance);
				}
			);
		}
	}
}
void AMonster::UpdateOptimizationLevel(float Significance)
{
	// 죽었거나 비활성 상태면 최적화 스킵
	if (IsDead() || !IsActive())
	{
		return;
	}

	// Significance 값에 따라 최적화 레벨 결정
	// 1.0 = 매우 가까움 (플레이어 근처)
	// 0.5 = 중간 거리
	// 0.0 = 매우 멀리

	if (Significance >= 0.6f) // 가까운 거리 - 최고 품질
	{
		// 모든 기능 활성화
		SetActorTickInterval(0.0f); // 매 프레임 Tick

		// AI 정상 작동
		if (m_AIControllerBase.IsValid() && m_AIControllerBase->GetBrainComponent())
		{
			if (m_AIControllerBase->GetBrainComponent()->IsPaused())
			{
				m_AIControllerBase->GetBrainComponent()->ResumeLogic(TEXT("High Significance"));
			}
		}

		// 애니메이션 정상 업데이트
		if (USkeletalMeshComponent* mesh = GetMesh())
		{
			mesh->bPauseAnims = false;
		}
	}
	else if (Significance >= 0.3f) // 중간 거리 - 중간 품질
	{
		// Tick 간격 늘림
		SetActorTickInterval(0.1f); // 0.1초마다 Tick (10fps)

		// AI는 유지하되 느리게 업데이트
		if (m_AIControllerBase.IsValid() && m_AIControllerBase->GetBrainComponent())
		{
			if (m_AIControllerBase->GetBrainComponent()->IsPaused())
			{
				m_AIControllerBase->GetBrainComponent()->ResumeLogic(TEXT("Medium Significance"));
			}
		}

		// 애니메이션 정상 업데이트
		if (USkeletalMeshComponent* mesh = GetMesh())
		{
			mesh->bPauseAnims = false;
		}
	}
	else // 먼 거리 - 최소 품질
	{
		// Tick 매우 느리게
		SetActorTickInterval(0.5f); // 0.5초마다 Tick (2fps)

		// AI 일시정지 (전투 중이 아닐 때만)
		if (m_AIControllerBase.IsValid() && m_AIControllerBase->GetBrainComponent())
		{
			// 적(플레이어)을 인지하지 않았다면 AI 일시정지
			ACharacterBase* Enemy = GetTarget();
			if (Enemy == nullptr)
			{
				m_AIControllerBase->GetBrainComponent()->PauseLogic(TEXT("Low Significance"));
			}
		}

		// 애니메이션은 유지 (멀리서도 보일 수 있으므로)
		// 단, VisibilityBasedAnimTickOption 설정으로 화면 밖이면 자동으로 안 그려짐
	}
}

void AMonster::deoptimize()
{
	// Significance Manager 등록 해제
	if (m_bUseSignificanceManager)
	{
		if (USignificanceManager* significanceManager = USignificanceManager::Get<USignificanceManager>(GetWorld()))
		{
			significanceManager->UnregisterObject(this);
		}
	}
}

void AMonster::PlayOnHitEffect(const FHitInformation& hitInfo)
{
	Super::PlayOnHitEffect(hitInfo);

	GetMesh()->SetScalarParameterValueOnMaterials(TEXT("DiffuseRedRatioOnHit"), 5.0f); // 바로 붉게 했다가,
	
	GetWorldTimerManager().ClearTimer(m_DiffuseRatioOnHitTimer);
	GetWorldTimerManager().SetTimer(m_DiffuseRatioOnHitTimer,
	[this]()
	{
		GetMesh()->SetScalarParameterValueOnMaterials(
		TEXT("DiffuseRedRatioOnHit"), 1.0f); // 다시 원래대로
	},
	0.25f, false
	);
}

void AMonster::SetBehaviorTreeFSMState(uint8 enumIndex) const
{
	m_AIControllerBase->GetBlackboardComponent()->SetValueAsEnum(AMonster::FSMStateKey, enumIndex);
}

void AMonster::SetIsDead(bool bIsDead)
{
	m_bIsDead = bIsDead;
	m_AIControllerBase->GetBlackboardComponent()->SetValueAsBool(TEXT("IsDead"), m_bIsDead);
}

void AMonster::Pause()
{
	GetMesh()->bPauseAnims = true;
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	
	m_AIControllerBase->StopBehaviorTree();
}

void AMonster::Unpause()
{
	GetMesh()->bPauseAnims = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	
	m_AIControllerBase->StartBehaviorTree();
}

ACharacterBase* AMonster::GetTarget() const
{
	return Cast<ACharacterBase>(m_AIControllerBase->GetBlackboardComponent()->GetValueAsObject(TEXT("Enemy")));
}


