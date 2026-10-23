#pragma once

#include "CoreMinimal.h"
#include "PuzzleBaseActor.h"
#include "PuzzleAction.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsAction.generated.h"

UCLASS()
class ESCAPEGAME260422_API APhysicsAction
    : public APuzzleBaseActor
    , public IPuzzleAction
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    TArray<AActor*> TargetActors;

    // 붕괴 시작점 오버라이드 (None이면 InstigatorActor 위치 사용)
    UPROPERTY(EditAnywhere, Category = "Collapse")
    AActor* CollapseOrigin = nullptr;

    // 전파 속도 (클수록 빠르게 퍼짐)
    UPROPERTY(EditAnywhere, Category = "Collapse")
    float PropagationSpeed = 300.f;

    // 딜레이 상한
    UPROPERTY(EditAnywhere, Category = "Collapse")
    float MaxDelay = 2.0f;

    // 랜덤 Impulse 세기
    UPROPERTY(EditAnywhere, Category = "Collapse")
    float ImpulseStrength = 300.f;

    // 위쪽 방향 Impulse 비율
    UPROPERTY(EditAnywhere, Category = "Collapse")
    float UpwardBias = 0.3f;

    virtual void ExecuteAction_Implementation(AActor* InstigatorActor) override;

private:
    void ActivateSingleActor(AActor* Actor);
};