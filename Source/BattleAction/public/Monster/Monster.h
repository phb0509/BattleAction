// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase/CharacterBase.h"
#include "Interfaces/PoolableActor.h"
#include "Templates/IsEnumClass.h"
#include "Templates/AndOrNot.h"
#include "Components/TimelineComponent.h"
#include "Templates/UnrealTypeTraits.h"
#include <type_traits>
#include "Monster.generated.h"

namespace MyProjectConcepts {
	template<typename T>
	concept ValidFSMStateType = 
		std::is_same_v<T, uint8> || 
		(std::is_enum_v<T> && sizeof(T) == sizeof(uint8));
}


UCLASS(Abstract)
class BATTLEACTION_API AMonster : public ACharacterBase,  public IPoolableActor
{
	GENERATED_BODY()

public:
	AMonster();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnDamage(const float damage, const bool bIsCriticalAttack, const FAttackInformation*, AActor* instigator, const FVector& causerLocation) override;

	virtual void OnGroggy();
	virtual void EndedGroggy();
	
	uint8 GetCurFSMState() const { return m_CurFSMState; }
	ACharacterBase* GetTarget() const;
	
	// template<
	// typename T, typename = typename TEnableIf<
	// TOr<
	// 		TIsSame<T, uint8>,
	// 		TAnd<
	// 			TIsEnumClass<T>,
	// 			TIntegralConstant<bool, sizeof(T) == sizeof(uint8)>
	// 		>
	// 	>::Value
	// >::Type>
	// void SetFSMState(const T state)
	// {
	// 	const uint8 enumIndex = static_cast<uint8>(state);
	// 	m_CurFSMState = enumIndex;
	// 	SetBehaviorTreeFSMState(enumIndex);
	// }
	
	template<MyProjectConcepts::ValidFSMStateType T>
	void SetFSMState(const T state)
	{
		const uint8 enumIndex = static_cast<uint8>(state);
		m_CurFSMState = enumIndex;
		SetBehaviorTreeFSMState(enumIndex);
	}

	
	
	void SetBehaviorTreeFSMState(uint8 enumIndex) const;
	void SetIsDead(bool bIsDead);

	void Pause();
	void Unpause();
	
protected:
	virtual void OnStaminaIsZero() override;
	
	virtual void Die() override;
	virtual void ExecEvent_EndedDeathMontage() override;

	// 사망시 호출할 TimeLine 이벤트.
	UFUNCTION()					
	void OnCalledTimelineEvent_Loop_AfterDeath(float curveValue); // 특정시간(조절 가능한)동안 디퓨즈값 검은색으로 점점 변환.

	UFUNCTION()
	void OnCalledTimelineEvent_End_AfterDeath(); // 타임라인 이벤트 종료시 호출(액터풀 회수직전 호출함수.)

	UFUNCTION()					
	void OnCalledTimelineEvent_Loop_DeathDissolve(float curveValue); // 특정시간(조절 가능한)동안 디퓨즈값 검은색으로 점점 변환.

	UFUNCTION()
	void OnCalledTimelineEvent_End_DeathDissolve(); // 타임라인 이벤트 종료시 호출(액터풀 회수직전 호출함수.)
	
	virtual void PlayOnHitEffect(const FHitInformation& hitInformation) override;
	
	// IPoolableActor VirtualFunction
	virtual void Initialize() override;
	virtual void Activate() override;
	virtual void Deactivate() override;
	virtual bool IsActive() override;
	
private:
	void setTimeline();

public:
	static const FName HomePosKey;
	static const FName PatrolPosKey;
	static const FName EnemyKey;
	static const FName FSMStateKey;
	static const FName IsCrowdControlState;

protected:
	uint8 m_CurFSMState;
	FTimerHandle m_DiffuseRatioOnHitTimer;

	FTimeline m_DeathTimeline;		
	FTimeline m_DeathDissolveTimeline;			

	UPROPERTY(EditAnywhere, Category = "Timeline | Death")
	TObjectPtr<UCurveFloat> m_DeathCurveFloat;

	UPROPERTY(EditAnywhere, Category = "Timeline | Death")
	TObjectPtr<UCurveFloat> m_DeathDissolveCurveFloat;
	
private:
	TWeakObjectPtr<AAIControllerBase> m_AIControllerBase;
	
};
