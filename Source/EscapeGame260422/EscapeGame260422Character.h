// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "C_InteractionComponent.h"
#include "InteractHintActor.h"
#include "PuzzleHighlightSubsystem.h"
#include "EscapeGame260422Character.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class AEscapeGame260422Character : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	AEscapeGame260422Character();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== Input Actions =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* LookAction;

	// V키: 퍼즐 하이라이트 토글
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* IA_ViewToggle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* IA_Grab;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* IA_Throw;

	// 뛰기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* IA_Sprint;

	//속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 700;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ExhaustedSpeed = 200.f;

	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);


	// ====== 스태미너 ======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaDrainRate = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRegenRate = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float ExhaustionRecoverThreshold = 0.3f; // 30% 회복 시 탈진 해제

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaIdleRegenMultiplier = 2.0f; // 정지 시 회복 속도 증가

	// 상태
	UPROPERTY(BlueprintReadOnly, Category = "Stamina")
	bool bIsSprinting = false;

	UPROPERTY(BlueprintReadOnly, Category = "Stamina")
	bool bIsExhausted = false;


	// 스태미너 비율(UI 용)
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetStaminaPercent() const;






public:
	// ===== 상호작용 컴포넌트 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	UC_InteractionComponent* InteractionComp;

	// 카메라 접근자 (InteractionComponent에서 카메라 방향 참조용)
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// ===== 밀기 이벤트 =====
	UFUNCTION()
	void OnStartPush();

	UFUNCTION()
	void OnStopPush();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPushing() const;

	// ===== 퍼즐 하이라이트 =====
	UFUNCTION(BlueprintCallable)
	void TogglePuzzleMode();

	// 하위 호환용 (더 이상 1인칭 전환과 묶지 않음)
	UFUNCTION(BlueprintCallable)
	void SetFirstPerson(bool bEnable);

	UFUNCTION(BlueprintCallable)
	void TogglePuzzleHighlight();

	UFUNCTION(BlueprintCallable)
	void UpdatePuzzleHighlightState();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsInPuzzleMode() const { return bPuzzleMode; }

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void ThrowObject();
	void StartAim();
	void StopAim();

private:
	// 게임 시작 시 모든 힌트 액터를 숨김 상태로 초기화
	void InitializeAllHintActors();

	float DefaultArmLength = 400.f;

	bool bIsFirstPerson = false;
	bool bPuzzleMode = false;
	bool bPuzzleHighlightMode = false;
};
