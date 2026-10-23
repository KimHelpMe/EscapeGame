#include "PressurePlate.h"

APressurePlate::APressurePlate()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = Box;

	// [핵심] Overlap 이벤트가 발생하려면 다음 조건들이 충족되어야 함:
	// 1. CollisionEnabled = QueryOnly 또는 QueryAndPhysics
	//    (Overlap만 필요하면 QueryOnly로 충분, 물리 충돌까지 필요하면 QueryAndPhysics)
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 2. ObjectType 설정
	Box->SetCollisionObjectType(ECC_WorldDynamic);

	// 3. Overlap 응답 명시
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	Box->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	// 4. GenerateOverlapEvents 활성화
	Box->SetGenerateOverlapEvents(true);

	Box->OnComponentBeginOverlap.AddDynamic(this, &APressurePlate::OnOverlapBegin);
	Box->OnComponentEndOverlap.AddDynamic(this, &APressurePlate::OnOverlapEnd);
}

void APressurePlate::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[PressurePlate] OnOverlapBegin: %s"),
		OtherActor ? *OtherActor->GetName() : TEXT("NULL"));

	if (!LinkedTrigger)
	{
		UE_LOG(LogTemp, Error, TEXT("[PressurePlate] LinkedTrigger가 연결되지 않음!"));
		return;
	}

	if (!OtherActor) return;

	LinkedTrigger->Activate(OtherActor);
}

void APressurePlate::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("[PressurePlate] OnOverlapEnd: %s"),
		OtherActor ? *OtherActor->GetName() : TEXT("NULL"));

	if (!bDeactivateOnExit) return;
	if (!LinkedTrigger) return;
	if (!OtherActor) return;

	// 아직 다른 액터가 올라가 있으면 비활성화 안 함
	TArray<AActor*> OverlappingActors;
	Box->GetOverlappingActors(OverlappingActors);
	if (OverlappingActors.Num() > 0) return;

	LinkedTrigger->Deactivate(OtherActor);
}