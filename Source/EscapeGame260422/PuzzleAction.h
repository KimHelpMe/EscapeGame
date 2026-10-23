#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PuzzleAction.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPuzzleAction : public UInterface
{
	GENERATED_BODY()
};


class ESCAPEGAME260422_API IPuzzleAction
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ExecuteAction(AActor* InstigatorActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DeactivateAction(AActor* InstigatorActor);

};
