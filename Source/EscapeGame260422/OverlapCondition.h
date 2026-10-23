// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "PuzzleBaseActor.h"
#include "Components/BoxComponent.h"
#include "PuzzleCondition.h"

#include "OverlapCondition.generated.h"

UCLASS()
class ESCAPEGAME260422_API AOverlapCondition : public APuzzleBaseActor, public IPuzzleCondition
{
	GENERATED_BODY()
	
public:	

	AOverlapCondition();

protected:

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Box;


public:	
	
	UPROPERTY(EditAnywhere)
	FName RequiredTag;

	virtual bool IsSatisfied_Implementation(AActor* InstigatorActor) const override;
};
