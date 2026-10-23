#include "OverlapCondition.h"

AOverlapCondition::AOverlapCondition()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);
	Box->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldDynamic);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	Box->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
}

bool AOverlapCondition::IsSatisfied_Implementation(AActor* InstigatorActor) const
{
	if (!Box) { UE_LOG(LogTemp, Error, TEXT("[OverlapCondition] Box null")); return false; }

	TArray<AActor*> Actors;
	Box->GetOverlappingActors(Actors);

	UE_LOG(LogTemp, Warning, TEXT("[OverlapCondition] 겹친 액터: %d개, RequiredTag: '%s'"),
		Actors.Num(), *RequiredTag.ToString());

	if (RequiredTag.IsNone())
		return Actors.Num() > 0;

	for (AActor* A : Actors)
	{
		if (A && A->ActorHasTag(RequiredTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("[OverlapCondition] 태그 매칭: %s"), *A->GetName());
			return true;
		}
	}
	return false;
}
