// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/DecalComponent.h"
#include "InteractHintActor.h"
#include "C_InteractionComponent.generated.h"

UENUM(BlueprintType)
enum class EInteractionState : uint8
{
	None,
	Grabbing,
	Pushing
};

UENUM(BlueprintType)
enum class EInteractType : uint8
{
	None,
	Grab,
	Push
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPEGAME260422_API UC_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UC_InteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== 상호작용 함수 =====
	UFUNCTION()
	void TryInteract();

	UFUNCTION()
	void Release();

	UFUNCTION()
	void Throw();

	UFUNCTION()
	void StartAiming();

	UFUNCTION()
	void StopAiming();

	UFUNCTION(BlueprintCallable)
	bool IsPushing() const { return CurrentState == EInteractionState::Pushing; }

	UFUNCTION(BlueprintCallable)
	bool IsGrabbing() const { return CurrentState == EInteractionState::Grabbing; }

	// ===== 설정값 =====

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float TraceDistance = 300.f;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float GrabMassLimit = 100.f;   // 기존 30 → 80으로 상향. 30kg 이하만 잡기는 너무 낮음

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float PushMassLimit = 500.f;  // 기존 200 → 500으로 상향

	UPROPERTY(EditAnywhere, Category = "Interaction|Grab")
	float HoldDistance = 200.f;

	// HoldHeightOffset: 카메라 기준 목표 Z에서 추가로 올리는 높이
	// 양수 = 카메라보다 위로, 음수 = 아래로
	// 3인칭에서 카메라가 이미 높이 있으므로 0~30 사이 권장
	UPROPERTY(EditAnywhere, Category = "Interaction|Grab")
	float HoldHeightOffset = 0.f;

	UPROPERTY(EditAnywhere, Category = "Interaction|Throw")
	float ThrowForce = 1000.f;

	// [추가] 밀기 거리 (기존 하드코딩 120.f를 변수로)
	UPROPERTY(EditAnywhere, Category = "Interaction|Push")
	float PushDistance = 120.f;

	UPROPERTY(EditAnywhere, Category = "Interaction|Decal")
	UMaterialInterface* LandingDecalMaterial = nullptr;

	FRotator GrabbedRotation;


public:



private:
	// ===== 내부 상태 =====
	EInteractionState CurrentState = EInteractionState::None;
	bool bIsGrabbing = false;
	bool bIsAiming = false;

	ECollisionResponse OriginalPawnResponse;

	FRotator LockedRotation;
	UPrimitiveComponent* PushedComponent = nullptr;

	FVector PrevGrabLocation = FVector::ZeroVector;
	bool bHasPrevLocation = false;

	// ===== 컴포넌트 참조 =====
	UPROPERTY()
	class UPhysicsHandleComponent* PhysicsHandle = nullptr;

	UPROPERTY()
	UDecalComponent* LandingDecalComp = nullptr;

	// ===== 내부 함수 =====
	bool Trace(FHitResult& OutHit);
	EInteractType GetInteractType(UPrimitiveComponent* Comp);

	void Grab(UPrimitiveComponent* HitComp);
	void Push(UPrimitiveComponent* HitComp, const FHitResult& Hit);

	void UpdateLandingIndicator();
	bool PredictLanding(FVector& OutLoc, FVector& OutNormal);



	AInteractHintActor* CurrentFocusedHint = nullptr;

};
