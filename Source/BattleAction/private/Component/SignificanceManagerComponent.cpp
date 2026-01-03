// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/SignificanceManagerComponent.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "SignificanceManager.h"
#include "Monster/Monster.h"

USignificanceManagerComponent::USignificanceManagerComponent()
{
	// Tick 불필요 (SignificanceManager가 알아서 Update 호출)
	PrimaryComponentTick.bCanEverTick = false;
}

void USignificanceManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* owner = GetOwner();
	check(owner != nullptr);

	// SignificanceManager 가져오기
	USignificanceManager* significanceManager = USignificanceManager::Get<USignificanceManager>(GetWorld());
	check(significanceManager != nullptr);

	// SignificanceManager에 Owner Actor 등록
	significanceManager->RegisterObject(
		owner,
		TEXT("SignificanceManaged"),
		// Significance 계산 람다
		[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& ViewPoint) -> float
		{
			AActor* owner = GetOwner();
			check(owner != nullptr);

			// 거리 계산 (가까울수록 높은 값)
			const float distance = FVector::Dist(owner->GetActorLocation(), ViewPoint.GetLocation());

			// 0.0(멀리) ~ 1.0(가까이)
			return FMath::Clamp(1.0f - (distance / m_MawDistance), 0.0f, 1.0f);
		},
		USignificanceManager::EPostSignificanceType::Sequential,
		// Significance 변경 콜백 람다
		[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float NewSignificance,
		       bool bFinal)
		{
			OnSignificanceChanged(NewSignificance);
		}
	);

	UE_LOG(LogTemp, Log, TEXT("SignificanceManagerComponent: Registered %s"), *owner->GetName());
}

void USignificanceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 필수! SignificanceManager에서 등록 해제 (안 하면 크래시)
	if (USignificanceManager* SignificanceManager = USignificanceManager::Get<USignificanceManager>(GetWorld()))
	{
		if (AActor* Owner = GetOwner())
		{
			SignificanceManager->UnregisterObject(Owner);
			
			UE_LOG(LogTemp, Log, TEXT("SignificanceManagerComponent: Unregistered %s"), *Owner->GetName());
		}
	}

	Super::EndPlay(EndPlayReason);
}

void USignificanceManagerComponent::OnSignificanceChanged(float newSignificance)
{
	AActor* owner = GetOwner();
	check(owner != nullptr);

	// Monster인 경우 추가 체크
	AMonster* monster = CastChecked<AMonster>(owner);
	if (monster->IsDead() || !monster->IsActive())
	{
		return;
	}

	// Significance 값에 따라 최적화 레벨 결정
	if (newSignificance >= m_HighSignificanceThreshold) // High (가까운 거리, 최고품질)
	{
		//UE_LOG(LogTemp, Warning, TEXT(" ====================== High =============== "));
		
		// 모든 기능 활성화
		owner->SetActorTickInterval(0.0f); // 매 프레임 Tick

		// Monster인 경우 AI 정상 작동

		// if (AAIController* aiController = Cast<AAIController>(monster->GetController()))
		// {
		// 	if (UBrainComponent* brainComponent = aiController->GetBrainComponent())
		// 	{
		// 		if (brainComponent->IsPaused())
		// 		{
		// 			brainComponent->ResumeLogic(TEXT("High Significance"));
		// 		}
		// 	}
		// }
	}
	else if (newSignificance >= m_MediumSignificanceThreshold) // Medium (중간 거리)
	{
		//UE_LOG(LogTemp, Warning, TEXT(" ====================== Medium =============== "));
		
		// Tick 간격 늘림
		owner->SetActorTickInterval(0.1f); // 0.1초마다 Tick

		// Monster인 경우 AI 유지

		// if (AAIController* aiController = Cast<AAIController>(monster->GetController()))
		// {
		// 	if (UBrainComponent* brainComponent = aiController->GetBrainComponent())
		// 	{
		// 		if (brainComponent->IsPaused())
		// 		{
		// 			brainComponent->ResumeLogic(TEXT("Medium Significance"));
		// 		}
		// 	}
		// }
	}
	else // Low (먼 거리)
	{
		//UE_LOG(LogTemp, Warning, TEXT(" ====================== Low =============== "));
		
		// Tick 매우 느리게
		owner->SetActorTickInterval(0.5f); // 0.5초마다 Tick

		// // Monster인 경우 AI 일시정지 (전투 중이 아닐 때만)
		// if (m_bPauseAIWhenLow)
		// {
		// 	// 적(플레이어)을 인지하지 않았다면 AI 일시정지
		// 	ACharacterBase* enemy = monster->GetTarget();
		// 	if (enemy == nullptr)
		// 	{
		// 		if (AAIController* aiController = Cast<AAIController>(monster->GetController()))
		// 		{
		// 			if (UBrainComponent* brainComponent = aiController->GetBrainComponent())
		// 			{
		// 				brainComponent->PauseLogic(TEXT("Low Significance"));
		// 			}
		// 		}
		// 	}
		// }
	}
}
