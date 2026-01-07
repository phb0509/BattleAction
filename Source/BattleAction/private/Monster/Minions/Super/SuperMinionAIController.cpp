// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Minions/Super/SuperMinionAIController.h"
#include "Monster/Minions/Super/SuperMinion.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"


ASuperMinionAIController::ASuperMinionAIController(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	m_TeamID = FGenericTeamId(4);
	
	initAssets();
}

void ASuperMinionAIController::BeginPlay()
{
	Super::BeginPlay();
}

void ASuperMinionAIController::OnPossess(APawn* pawn)
{
	Super::OnPossess(pawn);
	
	UE_LOG(LogTemp, Warning, TEXT("ASuperMinionAIController :: OnPossess"));

	m_Owner = CastChecked<ASuperMinion>(pawn);

	UAIPerceptionComponent* aiPerceptionComponent = GetAIPerceptionComponent();
	check(aiPerceptionComponent != nullptr);
	
	aiPerceptionComponent->Activate();
	aiPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), true);
	
	UBlackboardComponent* BlackboardComponent = Blackboard.Get();
	check(BlackboardComponent != nullptr);
	
	BlackboardComponent->InitializeBlackboard(*m_BehaviorTree->BlackboardAsset); 
	
	if (UseBlackboard(m_BlackboardData.Get(), BlackboardComponent))
	{
		Blackboard->SetValueAsVector(AMonster::HomePosKey, pawn->GetActorLocation());
	}
}

void ASuperMinionAIController::OnUnPossess()
{
	Super::OnUnPossess();

	UE_LOG(LogTemp, Warning, TEXT("ASuperMinionAIController :: OnUnPossess"));

	UAIPerceptionComponent* aiPerceptionComponent = GetAIPerceptionComponent();
	check(aiPerceptionComponent != nullptr);
	
	aiPerceptionComponent->Deactivate();
	aiPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
}

void ASuperMinionAIController::UpdatePerceptedTargetActor(AActor* actor, FAIStimulus const Stimulus)
{
	ACharacterBase* const perceivedCharacter = CastChecked<ACharacterBase>(actor);
	
	const int teamType = GetTeamAttitudeTowards(*actor);
	FString teamTypeName = "";

	switch (teamType) // 
	{
	case 0: // 아군
		{
			teamTypeName = "Friendly::";
		}
		break;

	case 1: // 중립
		{
			teamTypeName = "Neutral::";
		}
		break;

	case 2: // 적
		{
			teamTypeName = "Enemy::";

			const ACharacterBase* const enemyOnBlackBoard = Cast<ACharacterBase>(Blackboard->GetValueAsObject(AMonster::EnemyKey));

			if (enemyOnBlackBoard == nullptr) 
			{
				Blackboard->SetValueAsObject(AMonster::EnemyKey, perceivedCharacter);
			}
		}
		break;

	default:
		break;
	}

	// 로그용
	FString log = "'" + m_Owner->Tags[0].ToString() + "'" + " Sensing " + "'" + teamTypeName + perceivedCharacter->Tags[0].ToString() + "'";

	switch (Stimulus.Type)
	{
	case 0: // react to sight;
		{
			log += " Using 'SightPerception'";
			break;
		}

	case 1: // react to hearing;
		{
			log += " Using 'HearingPerception'";
			break;
		}

	default:
		break;
	}

	//UE_LOG(LogTemp, Warning, TEXT("%s"), *log);
}

void ASuperMinionAIController::initAssets()
{
	AAIController::SetGenericTeamId(m_TeamID);
	
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObject(TEXT("BehaviorTree'/Game/MonsterAsset/SuperMinion/BT_SuperMinion.BT_SuperMinion'"));
	if (BTObject.Succeeded())
	{
		m_BehaviorTree = BTObject.Object;
	}
	checkf(IsValid(m_BehaviorTree.Get()), TEXT("BehaviorTree isn't Valid"));

	static ConstructorHelpers::FObjectFinder<UBlackboardData> BBObject(TEXT("BlackboardData'/Game/MonsterAsset/SuperMinion/BB_SuperMinion.BB_SuperMinion'"));
	if (BBObject.Succeeded())
	{
		m_BlackboardData = BBObject.Object;
	}
	checkf(IsValid(m_BlackboardData.Get()), TEXT("BlackboardData IsValid(isn't Valid"));

	check(IsValid(m_AIPerceptionComponent));
	m_AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ASuperMinionAIController::UpdatePerceptedTargetActor);
}

ETeamAttitude::Type ASuperMinionAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const APawn* otherPawn = CastChecked<APawn>(&Other))
	{
		if (const IGenericTeamAgentInterface* teamAgent = CastChecked<IGenericTeamAgentInterface>(otherPawn->GetController()))
		{
			if (teamAgent->GetGenericTeamId() == FGenericTeamId(4))
			{
				return ETeamAttitude::Friendly; 
			}
			else
			{
				return ETeamAttitude::Hostile; 
			}
		}
	}
	
	return ETeamAttitude::Neutral; 
}