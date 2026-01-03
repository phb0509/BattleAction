

#include "CharacterBase/AnimInstanceBase.h"
#include "CharacterBase/CharacterBase.h"


UAnimInstanceBase::UAnimInstanceBase() :
	m_bIsLastMontagePlayInterrupted(false),
	m_CurSpeed(0.0f),
	m_bIsOnGround(false),
	m_bIsFalling(false),
	m_bIsFlying(false)
{
}

void UAnimInstanceBase::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	OnMontageStarted.AddDynamic(this, &UAnimInstanceBase::Exec_OnMontageStarted);
	OnMontageEnded.AddDynamic(this, &UAnimInstanceBase::Exec_OnMontageEnded);

	m_Owner = Cast<ACharacterBase>(TryGetPawnOwner());
}

void UAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (m_Owner.IsValid())
	{
		m_CurSpeed = m_Owner->GetVelocity().Size();
		m_bIsOnGround = m_Owner->GetCharacterMovement()->IsMovingOnGround();
		m_bIsFalling = m_Owner->GetCharacterMovement()->IsFalling();
		m_bIsFlying = m_Owner->GetCharacterMovement()->IsFlying();
	}
}

void UAnimInstanceBase::Exec_OnMontageStarted(UAnimMontage* montage) 
{
	const FName montageName = montage->GetFName();
	
	if (m_FuncsOnCalledMontageEvent.Contains(montageName))
	{
		m_FuncsOnCalledMontageEvent[montageName].funcOnCalledMontageStarted.ExecuteIfBound();
	}
}

void UAnimInstanceBase::Exec_OnMontageEnded(UAnimMontage* montage, bool bInterrupted)
{
	const FName montageName = montage->GetFName();
	m_bIsLastMontagePlayInterrupted = bInterrupted;
	
	if (m_FuncsOnCalledMontageEvent.Contains(montageName))
	{
		m_FuncsOnCalledMontageEvent[montageName].funcOnCalledMontageAllEnded.ExecuteIfBound();
		
		if (bInterrupted)
		{
			m_FuncsOnCalledMontageEvent[montageName].funcOnCalledMontageInterruptedEnded.ExecuteIfBound();
		}
		else
		{
			m_FuncsOnCalledMontageEvent[montageName].funcOnCalledMontageNotInterruptedEnded.ExecuteIfBound();
		}
	}
}

void UAnimInstanceBase::PlayMontage(const FName& montageName, float inPlayRate)
{
	if (m_Montages.Contains(montageName))
	{
		if(IsValid(m_Montages[montageName]))
		{
			Montage_Play(m_Montages[montageName].Get(), inPlayRate);
		}
	}
}

void UAnimInstanceBase::JumpToMontageSectionByIndex(const UAnimMontage* montage, int32 newSection)
{
	check(montage != nullptr);
	
	const FString section = FString::FromInt(newSection);
	Montage_JumpToSection(*section, montage);
}

void UAnimInstanceBase::JumpToMontageSectionByName(const UAnimMontage* montage, FName newSection)
{
	check(montage != nullptr);

	Montage_JumpToSection(newSection, montage);
}

bool UAnimInstanceBase::IsCurrentMontage(const FName& montageName)
{
	UAnimMontage* curMontage = GetCurrentActiveMontage();
	UAnimMontage* targetMontage = GetMontage(montageName);

	if ((curMontage != nullptr && targetMontage != nullptr) && curMontage == targetMontage)
	{
		return true;
	}
	
	return false;
}

FName UAnimInstanceBase::GetCurrentMontageName() const
{
	const UAnimMontage* montage = GetCurrentActiveMontage();

	return montage == nullptr ? "" : montage->GetFName();
}

float UAnimInstanceBase::GetMontagePlayTime(const UAnimMontage* montage) 
{
	check(montage != nullptr);
	
	const int32 frameCount = montage->GetNumberOfSampledKeys();
	
	return montage->GetTimeAtFrame(frameCount) / montage->RateScale;
}

UAnimMontage* UAnimInstanceBase::GetMontage(const FName& montageName)
{
	if (m_Montages.Contains(montageName))
	{
		if(IsValid(m_Montages[montageName]))
		{
			return m_Montages[montageName].Get();
		}
	}

	return nullptr;
}

void UAnimInstanceBase::AnimNotify_Pause()
{
	const UAnimMontage* montage = GetCurrentActiveMontage();
	
	if (montage != nullptr)
	{
		Montage_Pause(montage);
	}
}

void UAnimInstanceBase::AnimNotify_End_Death() const
{
	End_Death.Broadcast();
}
