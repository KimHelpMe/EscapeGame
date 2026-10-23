#include "PuzzleTriggerActor.h"
#include "PuzzleCondition.h"
#include "PuzzleAction.h"

void APuzzleTriggerActor::Activate(AActor* InstigatorActor)
{
	UE_LOG(LogTemp, Warning, TEXT("[PuzzleTrigger] Activate 호출됨. Instigator: %s"),
		InstigatorActor ? *InstigatorActor->GetName() : TEXT("NULL"));

	UE_LOG(LogTemp, Warning, TEXT("[PuzzleTrigger] Conditions: %d개, Actions: %d개"),
		Conditions.Num(), Actions.Num());

	// 1단계: 조건 검사
	for (AActor* Cond : Conditions)
	{
		if (!Cond)
		{
			UE_LOG(LogTemp, Error, TEXT("[PuzzleTrigger] Condition이 null → 에디터에서 연결 확인"));
			continue;
		}

		bool bImpl = Cond->GetClass()->ImplementsInterface(UPuzzleCondition::StaticClass());
		UE_LOG(LogTemp, Warning, TEXT("[PuzzleTrigger] Condition '%s' 인터페이스: %s"),
			*Cond->GetName(), bImpl ? TEXT("OK") : TEXT("IPuzzleCondition 미구현!"));

		if (!bImpl) continue;

		bool bOK = IPuzzleCondition::Execute_IsSatisfied(Cond, InstigatorActor);
		UE_LOG(LogTemp, Warning, TEXT("[PuzzleTrigger] Condition '%s' 결과: %s"),
			*Cond->GetName(), bOK ? TEXT("충족") : TEXT("미충족 → 중단"));

		if (!bOK) return;
	}

	// 2단계: 액션 실행
	UE_LOG(LogTemp, Warning, TEXT("[PuzzleTrigger] 조건 모두 충족 → Actions 실행"));

	for (AActor* Act : Actions)
	{
		if (!Act)
		{
			UE_LOG(LogTemp, Error, TEXT("[PuzzleTrigger] Action이 null → 에디터에서 연결 확인"));
			continue;
		}

		bool bImpl = Act->GetClass()->ImplementsInterface(UPuzzleAction::StaticClass());
		UE_LOG(LogTemp, Warning, TEXT("[PuzzleTrigger] Action '%s' 인터페이스: %s"),
			*Act->GetName(), bImpl ? TEXT("OK") : TEXT("IPuzzleAction 미구현!"));

		if (!bImpl) continue;

		UE_LOG(LogTemp, Warning, TEXT("[PuzzleTrigger] Action '%s' 실행 중..."), *Act->GetName());
		IPuzzleAction::Execute_ExecuteAction(Act, InstigatorActor);
	}
}

void APuzzleTriggerActor::Deactivate(AActor* InstigatorActor)
{
	for (AActor* Act : Actions)
	{
		if (!Act) continue;
		if (!Act->GetClass()->ImplementsInterface(UPuzzleAction::StaticClass())) continue;

		IPuzzleAction::Execute_DeactivateAction(Act, InstigatorActor);
	}
}