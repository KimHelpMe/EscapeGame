#include "PuzzleHighlightSubsystem.h"
#include "InteractHintActor.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/KismetMaterialLibrary.h"

void UPuzzleHighlightSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// MPC_Highlight를 경로로 직접 로드
	// BP_GameInstance를 통한 할당 방식이 불안정하여 코드에서 직접 처리
	HighlightCollection = LoadObject<UMaterialParameterCollection>(
		nullptr,
		TEXT("/Game/ThirdPerson/Blueprints/HintOutLineMaterial/MPC_Highlight.MPC_Highlight")
	);

	if (HighlightCollection)
	{
		UE_LOG(LogTemp, Log, TEXT("[PuzzleHighlightSubsystem] MPC_Highlight 로드 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[PuzzleHighlightSubsystem] MPC_Highlight 로드 실패! 경로 확인 필요"));
	}
}

void UPuzzleHighlightSubsystem::RegisterActor(AActor* Actor)
{
	if (!Actor) return;
	RegisteredActors.AddUnique(Actor);

	// 현재 하이라이트 상태 즉시 반영
	if (AInteractHintActor* Hint = Cast<AInteractHintActor>(Actor))
	{
		Hint->SetPuzzleActive(bHighlightEnabled);
	}
}

void UPuzzleHighlightSubsystem::UnregisterActor(AActor* Actor)
{
	RegisteredActors.Remove(Actor);
}

void UPuzzleHighlightSubsystem::SetHighlightEnabled(bool bEnabled)
{
	bHighlightEnabled = bEnabled;

	for (TWeakObjectPtr<AActor>& WeakActor : RegisteredActors)
	{
		if (!WeakActor.IsValid()) continue;

		if (AInteractHintActor* Hint = Cast<AInteractHintActor>(WeakActor.Get()))
		{
			Hint->SetPuzzleActive(bEnabled);
		}
	}

	// V키로 OFF시 응시 비율도 0으로 리셋
	if (!bEnabled)
	{
		SetFocusRatio(0.f);
	}
}

void UPuzzleHighlightSubsystem::SetFocusRatio(float NewRatio)
{
	CurrentFocusRatio = FMath::Clamp(NewRatio, 0.f, 1.f);

	// [임시 진단]
	UE_LOG(LogTemp, Warning, TEXT("[SetFocusRatio] 호출됨. Ratio: %f, Collection: %s"),
		CurrentFocusRatio,
		HighlightCollection ? TEXT("유효") : TEXT("NULL"));

	if (HighlightCollection)
	{
		UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;

		UE_LOG(LogTemp, Warning, TEXT("[SetFocusRatio] World: %s"),
			World ? TEXT("유효") : TEXT("NULL"));

		if (World)
		{
			UKismetMaterialLibrary::SetScalarParameterValue(
				World,
				HighlightCollection,
				FName("FocusRatio"),
				CurrentFocusRatio
			);
		}
	}
}