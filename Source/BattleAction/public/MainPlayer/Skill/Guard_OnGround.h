// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayableCharacter/Skill.h"
#include "Guard_OnGround.generated.h"


class UCameraShakeBase;
class USoundWave;
class UParticleSystem;

UCLASS()
class BATTLEACTION_API UGuard_OnGround : public USkill
{
	GENERATED_BODY()
	
public:
	UGuard_OnGround();

	virtual void Initialize() override;
	virtual void Execute() override;

protected:
	virtual bool CanExecuteSkill() const override;
	
private:
	void onParrySuccess(AActor* instigator);

private:
	UPROPERTY(EditAnywhere, Category = "Montage")
	TObjectPtr<UAnimMontage> m_GuardMontage;
	
	FTimerHandle m_ParryingTimer;

	UPROPERTY(EditAnywhere)
	float m_ParryingTime;
	
	UPROPERTY(EditAnywhere, Category = "ParryingEffect")
	TSubclassOf<UCameraShakeBase> m_ParryingShake;
	
	UPROPERTY(EditAnywhere, Category = "ParryingEffect")
	TObjectPtr<USoundWave> m_ParryingSound;
	
	UPROPERTY(EditAnywhere, Category = "Parrying Effect")
	TObjectPtr<UParticleSystem> m_ParryingHitParticle;
};
