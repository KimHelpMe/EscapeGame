#pragma once

#include "CoreMinimal.h"
#include "PuzzleBaseActor.h"
#include "PuzzleAction.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "JumpPadAction.generated.h"

UCLASS()
class ESCAPEGAME260422_API AJumpPadAction : public APuzzleBaseActor, public IPuzzleAction
{
    GENERATED_BODY()

public:
    AJumpPadAction();

    UPROPERTY(EditAnywhere)
    float LaunchPower = 1500.f;

    // 활성화
    bool bActivated = false;

    virtual void ExecuteAction_Implementation(AActor* InstigatorActor) override;

    virtual void DeactivateAction_Implementation(AActor* InstigatorActor) override;

protected:

    // 점프패드 감지용
    UPROPERTY(VisibleAnywhere)
    UBoxComponent* JumpBox;

    UFUNCTION()
    void OnJumpBoxOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);


};