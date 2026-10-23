#include "PhysicsAction.h"

void APhysicsAction::ExecuteAction_Implementation(AActor* InstigatorActor)
{
    // 붕괴 기준점 결정
    FVector Origin = CollapseOrigin
        ? CollapseOrigin->GetActorLocation()
        : (InstigatorActor ? InstigatorActor->GetActorLocation() : GetActorLocation());

    for (AActor* Actor : TargetActors)
    {
        if (!Actor) continue;

        // 거리 기반 딜레이 계산
        float Distance = FVector::Dist(Origin, Actor->GetActorLocation());
        float Delay = FMath::Clamp(Distance / PropagationSpeed, 0.0f, MaxDelay);

        // 딜레이 후 개별 활성화
        FTimerHandle Handle;
        FTimerDelegate Delegate;
        Delegate.BindUObject(this, &APhysicsAction::ActivateSingleActor, Actor);
        GetWorldTimerManager().SetTimer(Handle, Delegate, Delay, false);
    }
}

void APhysicsAction::ActivateSingleActor(AActor* Actor)
{
    if (!Actor) return;

    TArray<UPrimitiveComponent*> Comps;
    Actor->GetComponents<UPrimitiveComponent>(Comps);

    for (UPrimitiveComponent* Comp : Comps)
    {
        if (!Comp) continue;

        Comp->SetSimulatePhysics(true);

        // 랜덤 수평 방향 + UpwardBias 적용
        FVector RandDir = FVector(
            FMath::RandRange(-1.f, 1.f),
            FMath::RandRange(-1.f, 1.f),
            UpwardBias
        ).GetSafeNormal();

        Comp->AddImpulse(RandDir * ImpulseStrength, NAME_None, true);
    }
}