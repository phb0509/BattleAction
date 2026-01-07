// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayableCharacter/PlayableCharacter.h"
#include "MainPlayer.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDamageOverrideDelegate, AActor*)

struct FInputActionValue;
class UMainPlayerSkillComponent;
class USpringArmComponent;
class UCameraComponent;
class UBoxComponent;
class UMotionWarpingComponent;
class UInputMappingContext;
class UInputAction;
class UParticleSystem;


enum class EMainPlayerStates : uint8;
enum class ETriggerEvent : uint8;


UCLASS()
class BATTLEACTION_API AMainPlayer final: public APlayableCharacter
{
	GENERATED_BODY()

public:
	AMainPlayer();
	
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void OnDamage(const float damage, const bool bIsCriticalAttack, const FAttackInformation*, AActor* instigator, const FVector& causerLocation) override;
	virtual void PlayOnHitEffect(const FHitInformation& hitInformation) override;

	
private:
	void initAssets();
	void printLog() const;
	void playAttackEffect();
	void printLog();
	
public:
	FOnDamageOverrideDelegate OnDamageOverride;
	
	static const FName SwordColliderName;
	static const FName ShieldForAttackColliderName;
	static const FName ShieldForGuardColliderName;

	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true)) 
	TObjectPtr<UCapsuleComponent> m_SwordCollider;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> m_ShieldForAttackCollider;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> m_ShieldCollider;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true)) 
	TObjectPtr<UCapsuleComponent> m_ShieldBottomCollider;
	
	
};
