#include "JumpPadAction.h"

AJumpPadAction::AJumpPadAction()
{
    JumpBox = CreateDefaultSubobject<UBoxComponent>(TEXT("JumpBox"));
    JumpBox->SetupAttachment(RootComponent);
    JumpBox->SetBoxExtent(FVector(100.f, 100.f, 50.f));
    JumpBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    JumpBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    JumpBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    JumpBox->SetGenerateOverlapEvents(true);

    JumpBox->SetAbsolute(false, false, true);
    JumpBox->SetWorldScale3D(FVector(1.f, 1.f, 1.f));

    JumpBox->OnComponentBeginOverlap.AddDynamic(this, &AJumpPadAction::OnJumpBoxOverlap);
}

void AJumpPadAction::ExecuteAction_Implementation(AActor* InstigatorActor)
{
    bActivated = true;
}

void AJumpPadAction::OnJumpBoxOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!bActivated) return;

    if (ACharacter* Char = Cast<ACharacter>(OtherActor))
    {
        Char->LaunchCharacter(FVector(0, 0, LaunchPower), false, true);
    }
}

void AJumpPadAction::DeactivateAction_Implementation(AActor* InstigatorActor)
{
  
    bActivated = false;
}