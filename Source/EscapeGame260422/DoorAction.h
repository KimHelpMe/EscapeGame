#pragma once
#include "CoreMinimal.h"
#include "PuzzleAction.h"
#include "PuzzleBaseActor.h"
#include "DoorAction.generated.h"

UCLASS()
class ESCAPEGAME260422_API ADoorAction : public APuzzleBaseActor, public IPuzzleAction
{
	GENERATED_BODY()
public:
	ADoorAction();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Door")
	AActor* DoorActor;

	UPROPERTY(EditAnywhere, Category = "Door")
	FRotator OpenRotation;

	UPROPERTY(EditAnywhere, Category = "Door")
	float OpenDuration = 1.5f;

	virtual void ExecuteAction_Implementation(AActor* InstigatorActor) override;

private:
	bool bIsOpening = false;
	float ElapsedTime = 0.f;
	FRotator StartRotation;
};
