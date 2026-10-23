#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "PuzzleTriggerActor.h"
#include "PressurePlate.generated.h"

UCLASS()
class ESCAPEGAME260422_API APressurePlate : public AActor
{
	GENERATED_BODY()

public:
	APressurePlate();

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Box;

public:
	UPROPERTY(EditAnywhere)
	APuzzleTriggerActor* LinkedTrigger;

	UPROPERTY(EditAnywhere)
	bool bDeactivateOnExit = true; // 떠나면 비활성화 여부

protected:
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// EndOverlap도 바인딩 (로그 확인용)
	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
