#pragma once

#include "CoreMinimal.h"
#include "PuzzleBaseActor.h"
#include "PuzzleAction.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/LightComponent.h"
#include "InteractHintActor.generated.h"

UCLASS()
class ESCAPEGAME260422_API AInteractHintActor : public AActor
{
	GENERATED_BODY()

public:
	AInteractHintActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 퍼즐 모드(V키) 활성화 여부
	UFUNCTION(BlueprintCallable)
	void SetPuzzleActive(bool bActive);

	// 시선 집중으로 활성화
	UFUNCTION(BlueprintCallable)
	void ActivateHint();

	// 상태 초기화
	UFUNCTION(BlueprintCallable)
	void ResetHintState();

	// 포커스 업데이트 (시선 감지용, 필요 시 외부에서 호출)
	void UpdateFocus(bool bInFocused, float DeltaTime);

	// 1인칭 전용 가시성 (현재는 SetPuzzleActive와 분리)
	void SetFirstPersonVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsInteractable() const { return bInteractable; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsActivated() const { return bActivated; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Grab")
	bool bGrabbable = false;

	// [추가] 내부 글로우 머티리얼 시스템
	// 평상시(비활성)에는 MaterialBase, 응시로 활성화되면 MaterialGlow로 교체
	// 에디터에서 BP 자식 클래스에 두 머티리얼을 할당해야 동작
	UPROPERTY(EditAnywhere, Category = "Hint|Material")
	class UMaterialInterface* MaterialBase = nullptr;

	UPROPERTY(EditAnywhere, Category = "Hint|Material")
	class UMaterialInterface* MaterialGlow = nullptr;



	// 시선 누적 시간
	float FocusTime = 0.f;

	// 활성화에 필요한 시선 유지 시간
	UPROPERTY(EditAnywhere, Category = "Hint")
	float RequiredFocusTime = 5.0f;

	// [추가] 시선 떼면 감소하는 속도 (초당 감소량)
	// 예: 2.0이면 시선 떼고 1초 후 FocusTime이 2초만큼 줄어듦
	// RequiredFocusTime(5초) ÷ FocusDecayRate(2.0) = 2.5초 만에 0으로 리셋
	UPROPERTY(EditAnywhere, Category = "Hint")
	float FocusDecayRate = 2.0f;

	// [추가] 응시 비율 (0.0 ~ 1.0)
	// PostProcess Material로 전달되어 외곽선 강도 결정
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetFocusRatio() const { return RequiredFocusTime > 0.f ? FMath::Clamp(FocusTime / RequiredFocusTime, 0.f, 1.f) : 0.f; }

	bool bFirstPersonVisible = false;
	bool bActivated = false;
	bool bInteractable = false;


	// 상하좌우 이동
	// 이동 진폭 (X=좌우, Y=전후, Z=상하) - 0이면 해당 축 고정
	UPROPERTY(EditAnywhere, Category = "Float")
	FVector FloatAmplitude = FVector::ZeroVector;

	// 초당 왕복 속도
	UPROPERTY(EditAnywhere, Category = "Float")
	float FloatFrequency = 1.0f;

private:
	// BeginPlay에서 저장할 기준 위치
	FVector FloatOrigin;
	// 경과 시간
	float FloatTime = 0.f;

};