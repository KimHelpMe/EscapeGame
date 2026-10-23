// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzleBlock.h"
#include "PuzzleHighlightSubsystem.h"



// Sets default values
APuzzleBlock::APuzzleBlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void APuzzleBlock::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto Subsys = GI->GetSubsystem<UPuzzleHighlightSubsystem>())
		{
			Subsys->RegisterActor(this);
		}
	}
}

// Called every frame
void APuzzleBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// void APuzzleBlock::SetHighlight_Implementation(bool bEnable)
// {
// 	if (Mesh)
// 	{
// 		Mesh->SetRenderCustomDepth(bEnable);
// 	}
// }