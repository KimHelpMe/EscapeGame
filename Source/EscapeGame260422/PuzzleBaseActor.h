#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"

#include "PuzzleBaseActor.generated.h"

class UBillboardComponent;
class UBoxComponent;

UCLASS()
class ESCAPEGAME260422_API APuzzleBaseActor : public AActor
{
	GENERATED_BODY()

public:
	APuzzleBaseActor();

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
	UBillboardComponent* Billboard;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* ClickCollision;
};