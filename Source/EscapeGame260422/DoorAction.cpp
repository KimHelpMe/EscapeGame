#include "DoorAction.h"

ADoorAction::ADoorAction()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ADoorAction::ExecuteAction_Implementation(AActor* InstigatorActor)
{
	UE_LOG(LogTemp, Warning, TEXT("[DoorAction] ExecuteAction 호출. DoorActor: %s"),
		DoorActor ? *DoorActor->GetName() : TEXT("NULL - 에디터에서 연결 필요!"));

	if (!DoorActor) return;
	if (bIsOpening) return;

	bIsOpening = true;
	ElapsedTime = 0.f;
	StartRotation = DoorActor->GetActorRotation();

	UE_LOG(LogTemp, Warning, TEXT("[DoorAction] 문 열기 시작. From:%s To:%s"),
		*StartRotation.ToString(), *OpenRotation.ToString());

	SetActorTickEnabled(true);
}

void ADoorAction::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsOpening || !DoorActor) return;

	ElapsedTime += DeltaTime;
	float Alpha = FMath::Clamp(ElapsedTime / OpenDuration, 0.f, 1.f);
	float EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f);
	DoorActor->SetActorRotation(FMath::Lerp(StartRotation, OpenRotation, EasedAlpha));

	if (Alpha >= 1.f)
	{
		bIsOpening = false;
		ElapsedTime = 0.f;
		SetActorTickEnabled(false);
		UE_LOG(LogTemp, Warning, TEXT("[DoorAction] 문 열기 완료"));
	}
}
