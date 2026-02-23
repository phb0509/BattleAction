// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayableCharacter/Skill.h"
#include "EarthStrike_InAir.generated.h"

class UParticleSystem;
class USoundWave;

UCLASS()
class BATTLEACTION_API UEarthStrike_InAir : public USkill
{
	GENERATED_BODY()

public:
	UEarthStrike_InAir();

	virtual void Initialize() override;
	virtual void Execute(const FInputInfos& inputInfos) override;

	UFUNCTION()
	void OnLanded(const FHitResult& Hit);

	// 기존 타이머 콜백 (주석 처리)
	// void ExecEvent_WhenOnGround();

	void attack();
	void playEffect();

protected:
	virtual bool CanExecuteSkill() const override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Montage")
	TObjectPtr<UAnimMontage> m_FallingToGroundMontage;
	
	UPROPERTY(EditAnywhere, Category = "Montage")
	TObjectPtr<UAnimMontage> m_EarthStrikeMontage;

	// 기존 타이머 (주석 처리)
	// FTimerHandle m_Timer;

	UPROPERTY(EditAnywhere, Category = "Skill | AttackRangeRadius")
	float m_AttackRangeRadius;
	
	UPROPERTY(EditAnywhere, Category = "Skill | m_DownGravityScale (DownVelocity)")
	float m_DownGravityScale;
	
	UPROPERTY(EditAnywhere, Category = "Particle")
	TObjectPtr<UParticleSystem> m_Particle;
	
	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> m_CameraShake;
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundWave> m_Sound;
};


