#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PuzzleHighlightSubsystem.generated.h"

class UMaterialParameterCollection;

/**
 * V키 토글 시 모든 InteractHintActor를 일괄 제어하는 서브시스템
 * BeginPlay에서 자동 생성됨 (GameInstance 서브시스템)
 *
 * [추가] 응시 비율을 PostProcess Material에 전달하기 위해
 * Material Parameter Collection을 사용
 */
UCLASS()
class ESCAPEGAME260422_API UPuzzleHighlightSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 서브시스템 초기화 시 MPC 자동 로드 (BP 할당 방식 대신 코드에서 직접 처리)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 액터 등록 (InteractHintActor::BeginPlay에서 호출)
	UFUNCTION(BlueprintCallable)
	void RegisterActor(AActor* Actor);

	// 액터 해제 (EndPlay에서 호출)
	UFUNCTION(BlueprintCallable)
	void UnregisterActor(AActor* Actor);

	// V키 토글 시 호출 - 등록된 모든 액터에 활성화/비활성화 전달
	UFUNCTION(BlueprintCallable)
	void SetHighlightEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsHighlightEnabled() const { return bHighlightEnabled; }

	// [추가] 응시 비율 설정 (0.0 ~ 1.0)
	// InteractHintActor::UpdateFocus에서 호출되어 PostProcess에 전달됨
	UFUNCTION(BlueprintCallable)
	void SetFocusRatio(float NewRatio);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetFocusRatio() const { return CurrentFocusRatio; }

	// [추가] 에디터에서 할당할 Material Parameter Collection
	// PostProcess Material이 이 Collection의 FocusRatio 파라미터를 읽음
	UPROPERTY(EditAnywhere, Category = "Highlight")
	UMaterialParameterCollection* HighlightCollection = nullptr;

private:
	// TWeakObjectPtr 사용으로 GC 안전하게 관리
	TArray<TWeakObjectPtr<AActor>> RegisteredActors;

	bool bHighlightEnabled = false;

	// [추가] 현재 응시 비율 (0.0 ~ 1.0)
	float CurrentFocusRatio = 0.f;
};