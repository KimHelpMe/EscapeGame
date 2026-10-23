// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleBaseActor.h"
#include "PuzzleAction.h"
#include "PuzzleCondition.h"
#include "Components/BillboardComponent.h"
#include "PuzzleTriggerActor.generated.h"

UCLASS()
class ESCAPEGAME260422_API APuzzleTriggerActor : public APuzzleBaseActor
{
	GENERATED_BODY()
	
public:	
	
	UPROPERTY(EditAnywhere, Category = "Puzzle",
		meta = (MustImplement = "PuzzleCondition"))
	TArray<AActor*> Conditions;

	UPROPERTY(EditAnywhere, Category = "Puzzle",
		meta = (MustImplement = "PuzzleAction"))
	TArray<AActor*> Actions;

	UFUNCTION(BlueprintCallable)
	void Activate(AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable)
	void Deactivate(AActor* InstigatorActor);

};
