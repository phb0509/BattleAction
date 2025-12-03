#include "GameSystem/MonsterSpawnerManager.h"

#include "LevelEditor.h"
#include "GameSystem/MonsterSpawner.h"
#include "Components/BoxComponent.h"
#include "PlayableCharacter/PlayableCharacter.h"
#include "Monster/Monster.h"
#include "SubSystems/ActorPoolManager.h"
#include "ActorPool/ActorPool.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

AMonsterSpawnerManager::AMonsterSpawnerManager()
{
#if WITH_EDITOR
    // 에디터에서만 Tick 활성화
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
#else
    // 런타임에서는 Tick 비활성화
    PrimaryActorTick.bCanEverTick = false;
#endif

    // 루트 컴포넌트로 트리거 박스 생성
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    
    TriggerVolume->SetBoxExtent(FVector(200.f, 200.f, 100.f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    TriggerVolume->SetHiddenInGame(true); // 게임에선 숨기고, 에디터에서만 보이게
    TriggerVolume->ShapeColor = FColor::Green; // 와이어프레임 색깔
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AMonsterSpawnerManager::OnBeginOverlap);

    FCoreDelegates::OnPreExit.AddUObject(this, &AMonsterSpawnerManager::SaveSpawnerDataToDisk);
}

void AMonsterSpawnerManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("AMonsterSpawnerManager :: BeginPlay"));
    
}

void AMonsterSpawnerManager::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APlayableCharacter* player = Cast<APlayableCharacter>(OtherActor);
    if (player == nullptr)
    {
       return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AMonsterSpawnerManager :: OnBeginOverlap"));
    
    // 몬스터 스폰
    spawnMonsters();
}

void AMonsterSpawnerManager::spawnMonsters()
{
    UE_LOG(LogTemp, Warning, TEXT("AMonsterSpawnerManager :: spawnMonsters"));
}

#if WITH_EDITOR

bool AMonsterSpawnerManager::ShouldTickIfViewportsOnly() const
{
    return true;
}

void AMonsterSpawnerManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    UE_LOG(LogTemp, Warning, TEXT("PostEditChangeProperty :: First"));
    
    if (PropertyChangedEvent.Property)
    {
       FName PropertyName = PropertyChangedEvent.GetPropertyName();
        
       if (PropertyName == GET_MEMBER_NAME_CHECKED(AMonsterSpawnerManager, m_MonsterSpawners))
       {
          // 배열에 어떤 변화가 일어났는지 확인
          EPropertyChangeType::Type changeType = PropertyChangedEvent.ChangeType;
            
          switch (changeType)
          {
          case EPropertyChangeType::ArrayAdd:
             UE_LOG(LogTemp, Warning, TEXT("Array element added!"));
             handleArrayAdd(PropertyChangedEvent);
             break;
                    
          case EPropertyChangeType::ArrayRemove:
             UE_LOG(LogTemp, Warning, TEXT("Array element removed!"));
             handleArrayRemove(PropertyChangedEvent);
             break;
                    
          case EPropertyChangeType::ArrayClear:
             UE_LOG(LogTemp, Warning, TEXT("Array cleared!"));
             handleArrayClear();
             break;
                    
          case EPropertyChangeType::ValueSet:
             UE_LOG(LogTemp, Warning, TEXT("Array element value changed!"));
             break;
                    
          case EPropertyChangeType::Duplicate:
             UE_LOG(LogTemp, Warning, TEXT("Array element duplicated!"));
             handleArrayAdd(PropertyChangedEvent);
             break;
                    
          default:
             break;
          }
       }
    }
}

void AMonsterSpawnerManager::PreEditChange(FProperty* PropertyAboutToChange)
{
    Super::PreEditChange(PropertyAboutToChange);
    
    UE_LOG(LogTemp, Warning, TEXT("AMonsterSpawnerManager :: PreEditChange"));
    int32 size = m_MonsterSpawners.Num();
    
    if (size >= 0)
    {
        int temp = 0;
    }
    
}


void AMonsterSpawnerManager::handleArrayAdd(FPropertyChangedEvent& PropertyChangedEvent)
{
    // 어느 인덱스에 추가되었는지 확인
    int32 arrayIndex = PropertyChangedEvent.GetArrayIndex(TEXT("m_MonsterSpawners"));
    
    UE_LOG(LogTemp, Warning, TEXT("Added at index: %d"), arrayIndex);

    // 새로 추가된 요소에 대해 씬컴포넌트 생성
    if (m_MonsterSpawners.IsValidIndex(arrayIndex))
    {
        FString componentName = FString::Printf(TEXT("SpawnPoint_%d"), arrayIndex);
        UMonsterSpawner* monsterSpawner = NewObject<UMonsterSpawner>(
            this,
            UMonsterSpawner::StaticClass(),
            *componentName
        );

        if (monsterSpawner != nullptr)
        {
            monsterSpawner->SetupAttachment(RootComponent);
            monsterSpawner->RegisterComponent();
            monsterSpawner->CreationMethod = EComponentCreationMethod::Instance;
            monsterSpawner->bIsEditorOnly = true;
            monsterSpawner->SetHiddenInGame(true);
            monsterSpawner->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
            monsterSpawner->SetOwnerManager(this);
            
            m_MonsterSpawners[arrayIndex] = monsterSpawner;
        }
    }

    refreshEditorView();

    // 저장 필요 표시
    const bool temp = MarkPackageDirty();
}

void AMonsterSpawnerManager::handleArrayRemove(FPropertyChangedEvent& PropertyChangedEvent)
{
    UE_LOG(LogTemp, Warning, TEXT("handleArrayRemove"));
    
    int32 removedIndex = PropertyChangedEvent.GetArrayIndex(TEXT("m_MonsterSpawners"));
    
    m_MonsterSpawners[removedIndex]->UnregisterComponent();
    m_MonsterSpawners[removedIndex]->DestroyComponent();
    m_MonsterSpawners[removedIndex]->ConditionalBeginDestroy();
    m_MonsterSpawners.RemoveAtSwap(removedIndex);
    
    refreshEditorView();
}

void AMonsterSpawnerManager::handleArrayClear()
{
    UE_LOG(LogTemp, Warning, TEXT("MonsterSpawnersArray Cleaned Up!!"));

    for (int32 i = 0; i < m_MonsterSpawners.Num(); ++i)
    {
        UMonsterSpawner* monsterSpawner = m_MonsterSpawners[i];

        if (IsValid(monsterSpawner))
        {
            monsterSpawner->UnregisterComponent();
            monsterSpawner->DestroyComponent();
            monsterSpawner->ConditionalBeginDestroy();
        }

        m_MonsterSpawners.RemoveAtSwap(i);
    }

    refreshEditorView();
}

void AMonsterSpawnerManager::refreshMonsterSpawners()
{
    for (int32 i = m_MonsterSpawners.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(m_MonsterSpawners[i]))
        {
            m_MonsterSpawners.RemoveAt(i);  
        }
    }
}

void AMonsterSpawnerManager::RemoveMonsterSpawner(const FName& componentName) 
{
    for (int32 i = m_MonsterSpawners.Num() - 1; i >= 0; --i)
    {
        if (m_MonsterSpawners[i].GetName() == componentName)
        {
            m_MonsterSpawners.RemoveAt(i);  
        }
    }

    refreshEditorView();
}

void AMonsterSpawnerManager::refreshEditorView() const
{
    if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
    {
        FLevelEditorModule& LevelEditor = 
           FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
        LevelEditor.BroadcastComponentsEdited();
    }
    
    // 저장 필요 표시
    const bool temp = MarkPackageDirty();
}

void AMonsterSpawnerManager::PostLoad() 
{
    Super::PostLoad();
    //
    //     // 기존에 있던 컴포넌트 깨끗이 제거
    //     for (UMonsterSpawner* SpawnComp : SpawnedMonsterSpawners)
    //     {
    //         if (SpawnComp)
    //         {
    //             SpawnComp->DestroyComponent();
    //         }
    //     }
    //     SpawnedMonsterSpawners.Empty();
    //
    //     // 구조체 데이터 기반으로 컴포넌트 새로 생성
    //     for (int32 i = 0; i < SpawnerSavedDataArray.Num(); ++i)
    //     {
    //         const FSpawnerSavedData& Data = SpawnerSavedDataArray[i];
    //
    //         FString CompName = FString::Printf(TEXT("SpawnPoint_%d"), i);
    //         UMonsterSpawner* NewSpawner = NewObject<UMonsterSpawner>(this, UMonsterSpawner::StaticClass(), *CompName);
    //         if (NewSpawner)
    //         {
    //             NewSpawner->SetupAttachment(RootComponent);
    //             NewSpawner->CreationMethod = EComponentCreationMethod::Instance;
    //             NewSpawner->bIsEditorOnly = true;
    //             NewSpawner->RegisterComponent();
    //
    //             NewSpawner->SetRelativeLocation(Data.RelativeLocation);
    //             NewSpawner->m_MonsterClass = Data.MonsterClass;
    //             NewSpawner->m_SpawnInterval = Data.SpawnInterval;
    //             NewSpawner->m_MaxSpawnCount = Data.MaxSpawnCount;
    //
    //             SpawnedMonsterSpawners.Add(NewSpawner);
    //         }
    //     }
}

void AMonsterSpawnerManager::SaveSpawnerDataToDisk()
{
    // #if WITH_EDITOR
    //     SpawnerSavedDataArray.Empty();
    //     
    //     for (UMonsterSpawner* SpawnComp : m_MonsterSpawners)
    //     {
    //         if (SpawnComp && IsValid(SpawnComp))
    //         {
    //             FSpawnerSavedData Data;
    //             Data.MonsterClass = SpawnComp->GetM_MonsterClass();
    //             Data.SpawnInterval = SpawnComp->GetM_SpawnInterval();
    //             Data.MaxSpawnCount = SpawnComp->GetM_MaxSpawnCount();
    //             Data.RelativeLocation = SpawnComp->GetRelativeLocation();
    //
    //             SpawnerSavedDataArray.Add(Data);
    //         }
    //     }
    //
    //     MarkPackageDirty();  // 에디터에 저장 필요 표시
    // #endif
}
#endif
