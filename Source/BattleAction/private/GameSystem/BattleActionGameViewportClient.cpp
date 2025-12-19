// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystem/BattleActionGameViewportClient.h"
#include "SignificanceManager.h"
#include "Kismet/GameplayStatics.h"

void UBattleActionGameViewportClient::Tick(float DeltaTime)
{
	// 상위 클래스의 Tick 함수를 호출합니다.
	Super::Tick(DeltaTime);

	// 유효한 월드 및 시그니피컨스 매니저 인스턴스가 있는지 확인합니다.
	if (UWorld* CurrentWorld = GetWorld())
	{
		if (USignificanceManager* SignificanceManager = USignificanceManager::Get<USignificanceManager>(CurrentWorld))
		{
			// 프레임당 한 번 업데이트하며, 플레이어 0 의 월드 트랜스폼만 사용합니다.
			if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(CurrentWorld, 0))
			{
				// 시그니피컨스 매니저가 배열 뷰를 사용합니다. 엘리먼트가 하나인 배열을 생성하여 트랜스폼을 저장합니다.
				TArray<FTransform> TransformArray;
				TransformArray.Add(PlayerPawn->GetTransform());
				// 시그니피컨스 매니저를 배열 뷰를 통해 엘리먼트가 하나인 배열로 업데이트합니다.
				SignificanceManager->Update(TArrayView<FTransform>(TransformArray));
				
			}
		}
	}
}

