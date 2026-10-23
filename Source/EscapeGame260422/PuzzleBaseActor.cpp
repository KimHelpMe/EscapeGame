#include "PuzzleBaseActor.h"

#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "UObject/ConstructorHelpers.h"


APuzzleBaseActor::APuzzleBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;

	//root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	//Billboard
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Bilboard"));
	Billboard->SetupAttachment(Root);

	Billboard->SetHiddenInGame(true); // InGameHidden
	Billboard->SetVisibility(true); // Visibility In Edit
	Billboard->SetIsVisualizationComponent(true);

	//Normal Engine Icon
	static ConstructorHelpers::FObjectFinder<UTexture2D> Sprite(TEXT("/Engine/EditorResources/S_Actor"));
	if (Sprite.Succeeded())
	{
		Billboard->Sprite = Sprite.Object;
	}
	
	//ClickCollision
	ClickCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ClickCollision"));
	ClickCollision->SetupAttachment(Root);

	ClickCollision->SetBoxExtent(FVector(40.f));
	ClickCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ClickCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	ClickCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ClickCollision->SetHiddenInGame(true);
}
