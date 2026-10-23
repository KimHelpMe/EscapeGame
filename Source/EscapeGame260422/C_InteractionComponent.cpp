// Fill out your copyright notice in the Description page of Project Settings.

#include "C_InteractionComponent.h"
#include "EscapeGame260422Character.h"
#include "Camera/CameraComponent.h"

#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"



UC_InteractionComponent::UC_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	OriginalPawnResponse = ECR_Block;
}


void UC_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();

	if (PhysicsHandle)
	{
		// [수정 1] 잡기 안정화 - 강성/감쇠 균형 조정
		// 기존: LinearDamping=150, LinearStiffness=800 → 물체가 목표 지점에 과도하게 반응해 요동
		// 수정: Damping을 높여서 진동을 빠르게 수렴시키고, Stiffness를 낮춰 급격한 힘 방지
		PhysicsHandle->LinearDamping = 2000.f;
		PhysicsHandle->LinearStiffness = 7500.f;

		PhysicsHandle->AngularDamping = 2000.f;
		PhysicsHandle->AngularStiffness = 8000.f;

		// 보간 속도 - 물체가 목표를 따라가는 속도
		PhysicsHandle->InterpolationSpeed = 100.f;
	}

	LandingDecalComp = NewObject<UDecalComponent>(GetOwner());
	LandingDecalComp->RegisterComponent();
	LandingDecalComp->AttachToComponent(GetOwner()->GetRootComponent(),
		FAttachmentTransformRules::KeepWorldTransform);

	LandingDecalComp->SetVisibility(false);

	if (LandingDecalMaterial)
	{
		LandingDecalComp->SetDecalMaterial(LandingDecalMaterial);
	}

	LandingDecalComp->DecalSize = FVector(20.f, 40.f, 40.f);
}

void UC_InteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// =====================================================
	// [수정 2] 밀기(Push) 안정화
	// 기존 문제: 캐릭터가 AddMovementInput으로 직접 이동하면서
	//           PhysicsHandle 타겟과 충돌 → 물체가 튕기거나 캐릭터가 막힘
	// 수정: 캐릭터 이동 대신 PhysicsHandle로만 물체를 이동,
	//       캐릭터는 물체와의 거리를 유지하는 방식으로 변경
	// =====================================================
	if (CurrentState == EInteractionState::Pushing && PhysicsHandle && PhysicsHandle->GrabbedComponent)
	{
		// 밀기 중 캐릭터 회전 고정
		GetOwner()->SetActorRotation(LockedRotation);

		UPrimitiveComponent* GrabbedComp = PhysicsHandle->GrabbedComponent;

		FVector CharLoc = GetOwner()->GetActorLocation();
		FVector Forward = GetOwner()->GetActorForwardVector();

		// 물체 목표 위치: 캐릭터 앞 일정 거리, Z는 물체 자체 높이 유지
		FVector TargetLoc = CharLoc + (Forward * PushDistance);
		TargetLoc.Z = GrabbedComp->GetComponentLocation().Z;

		// 부드러운 목표 이동 (급격한 이동 방지)
		FVector CurrentTargetLoc = GrabbedComp->GetComponentLocation();
		FVector SmoothTarget = FMath::VInterpTo(CurrentTargetLoc, TargetLoc, DeltaTime, 8.f);

		PhysicsHandle->SetTargetLocation(SmoothTarget);

		// [핵심 수정] AddMovementInput 제거
		// 기존: Char->AddMovementInput(Forward, 0.5f) → 캐릭터가 물체 안으로 파고듦
		// 수정: 물체가 캐릭터를 밀도록 물리에 맡기고, 캐릭터는 이동하지 않음
		// 플레이어가 직접 W키로 전진하면 자연스럽게 물체를 미는 효과 발생
	}


	// =====================================================
	// [수정 3] 잡기(Grab) 안정화
	// =====================================================
	// 설계: 옵션 C + 방법 2
	//   - 수평 위치: 캐릭터 위치 + (카메라 Yaw 방향) * HoldDistance
	//     → 카메라 위치를 쓰면 SpringArm Probe / 캡슐 충돌과 양성 피드백 루프가 발생
	//     → 캐릭터 위치 기준으로 두면 캡슐 앞 빈 공간이 안정적으로 확보됨
	//   - 높이: 기본 높이 + HoldHeightOffset + (카메라 Pitch에 따른 Z 오프셋)
	//     → Pitch는 위치 계산의 수평 방향에는 반영하지 않음 (캡슐 충돌 회피)
	//     → 위아래 시선에 따라 물체가 들렸다 내려가는 효과만 부여
	// =====================================================
	if (CurrentState == EInteractionState::Grabbing && PhysicsHandle && PhysicsHandle->GrabbedComponent)
	{
		APawn* Pawn = Cast<APawn>(GetOwner());
		if (!Pawn) return;

		AController* Controller = Pawn->GetController();
		if (!Controller) return;

		const FRotator ControlRot = Controller->GetControlRotation();

		// Yaw만 추출 → 수평 방향용 Forward
		const FRotator YawOnlyRot(0.f, ControlRot.Yaw, 0.f);
		const FVector YawForward = YawOnlyRot.Vector();

		// Pitch에 따른 높이 오프셋
		// ControlRot.Pitch는 위를 보면 음수, 아래를 보면 양수로 들어오는 경우가 일반적이지만
		// UE 컨트롤러 회전은 위쪽이 양수(0~90), 아래쪽이 음수(0~-90, 또는 270~360 wrap)로 옴.
		// 안정적으로 처리하기 위해 NormalizeAxis로 -180~180 범위로 정규화.
		const float NormalizedPitch = FRotator::NormalizeAxis(ControlRot.Pitch);

		// HeightSensitivity: Pitch 1도당 Z 변화량
		// 예: 1.5라면 Pitch 60도에서 +90, -60도에서 -90
		const float HeightSensitivity = 1.5f;
		float PitchHeight = NormalizedPitch * HeightSensitivity;

		// 위아래 범위 클램프 (캐릭터 발 밑이나 머리 위 너무 멀리 가지 않도록)
		PitchHeight = FMath::Clamp(PitchHeight, -50.f, 150.f);

		// 캐릭터 기준 위치
		const FVector CharLoc = GetOwner()->GetActorLocation();

		// 기본 높이 (캐릭터 루트가 발 밑 기준일 때 가슴/머리 정도 높이로 띄움)
		const float BaseHeight = 80.f;

		const FVector DesiredLoc = CharLoc
			+ YawForward * HoldDistance
			+ FVector(0.f, 0.f, BaseHeight + HoldHeightOffset + PitchHeight);

		UPrimitiveComponent* GrabbedComp = PhysicsHandle->GrabbedComponent;
		const FVector CurrentLoc = GrabbedComp->GetComponentLocation();

		// 보간으로 부드럽게 따라오게
		const FVector FinalLoc = FMath::VInterpTo(CurrentLoc, DesiredLoc, DeltaTime, 20.f);

		PhysicsHandle->SetTargetLocation(FinalLoc);

		// 회전: 카메라 Yaw 방향으로 정렬 (Pitch/Roll 0)
		FRotator TargetRot = ControlRot;
		TargetRot.Pitch = 0.f;
		TargetRot.Roll = 0.f;
		PhysicsHandle->SetTargetRotation(TargetRot);

		// 이전 위치 갱신
		PrevGrabLocation = CurrentLoc;
		bHasPrevLocation = true;
	}

	// 데칼 업데이트
	if (bIsAiming)
		UpdateLandingIndicator();
	else if (LandingDecalComp)
		LandingDecalComp->SetVisibility(false);

	// 응시 
	FHitResult Hit;
	bool bHit = Trace(Hit);

	AInteractHintActor* NewHint = nullptr;

	if (bHit)
	{
		NewHint = Cast<AInteractHintActor>(Hit.GetActor());
	}

	// 이전 대상이랑 다르면
	if (CurrentFocusedHint != NewHint)
	{
		// 이전 힌트 포커스 해제
		if (CurrentFocusedHint)
		{
			CurrentFocusedHint->UpdateFocus(false, DeltaTime);
		}

		CurrentFocusedHint = NewHint;
	}

	// 현재 보고 있는 힌트 처리
	if (CurrentFocusedHint)
	{
		CurrentFocusedHint->UpdateFocus(true, DeltaTime);
	}



}

// ===================================================
// 시스템 핵심
// ===================================================

bool UC_InteractionComponent::Trace(FHitResult& OutHit)
{
	FVector Start;
	FRotator Rot;
	GetOwner()->GetActorEyesViewPoint(Start, Rot);

	FVector End = Start + (Rot.Vector() * TraceDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	FCollisionShape Sphere = FCollisionShape::MakeSphere(8.f);

	return GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, ECC_Visibility, Sphere, Params);
}

EInteractType UC_InteractionComponent::GetInteractType(UPrimitiveComponent* Comp)
{
	if (!Comp) return EInteractType::None;

	AActor* Owner = Comp->GetOwner();
	if (!Owner) return EInteractType::None;

	// 퍼즐 모드 상태 가져오기
	bool bPuzzleMode = false;
	if (AEscapeGame260422Character* Char = Cast<AEscapeGame260422Character>(GetOwner()))
	{
		bPuzzleMode = Char->IsInPuzzleMode();
	}

	//  [핵심] PuzzleOnly인데 V 안 켜져 있으면 차단
	if (Owner->ActorHasTag("PuzzleOnly") && !bPuzzleMode)
	{
		return EInteractType::None;
	}


	if (Owner->ActorHasTag("Interact.Fixed"))
		return EInteractType::None;

	float Mass = Comp->GetMass();

	if (Owner->ActorHasTag("Interact.Grab"))
		return EInteractType::Grab;

	if (Owner->ActorHasTag("Interact.Push"))
		return EInteractType::Push;


	if (Mass < GrabMassLimit)
		return EInteractType::Grab;

	if (Mass < PushMassLimit)
		return EInteractType::Push;

	return EInteractType::None;
}


void UC_InteractionComponent::TryInteract()
{
	if (CurrentState != EInteractionState::None)
		return;

	FHitResult Hit;
	if (!Trace(Hit)) return;

	// 1. Hint 체크 - Interactable 상태면 그랩 시도
	if (AInteractHintActor* Hint = Cast<AInteractHintActor>(Hit.GetActor()))
	{
		// bGrabbable이면 IsInteractable 체크 없이 바로 그랩
		if (Hint->bGrabbable && Hint->Mesh && Hint->bFirstPersonVisible)
		{
			Hint->Mesh->SetMobility(EComponentMobility::Movable);
			Hint->Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Hint->Mesh->SetSimulatePhysics(true);
			Hint->Mesh->SetEnableGravity(false);

			UE_LOG(LogTemp, Warning, TEXT("[Interact] HintActor Grab 시도. SimPhysics: %s"),
				Hint->Mesh->IsSimulatingPhysics() ? TEXT("true") : TEXT("false"));

			Grab(Hint->Mesh);
			return;
		}

		// [수정] 죽은 코드 제거
		// 기존: if (!Hint->IsInteractable()) return; return;
		// → 분기 결과가 동일했음. 비-그랩 힌트는 TryInteract로 처리하지 않으므로 그대로 반환.
		return;
	}

	// 2. 일반 물리 오브젝트 처리
	UPrimitiveComponent* Comp = Hit.GetComponent();
	if (!Comp || !Comp->IsSimulatingPhysics())
		return;

	EInteractType Type = GetInteractType(Comp);

	switch (Type)
	{
	case EInteractType::Grab:
		Grab(Comp);
		break;
	case EInteractType::Push:
		Push(Comp, Hit);
		break;
	}




}



void UC_InteractionComponent::Grab(UPrimitiveComponent* HitComp)
{

	if (!PhysicsHandle || !HitComp) return;

	// 물리가 꺼져있어도 강제로 활성화
	if (!HitComp->IsSimulatingPhysics())
	{
		HitComp->SetMobility(EComponentMobility::Movable);
		HitComp->SetSimulatePhysics(true);
		HitComp->SetEnableGravity(false);
	}

	CurrentState = EInteractionState::Grabbing;
	bIsGrabbing = true;


	OriginalPawnResponse = HitComp->GetCollisionResponseToChannel(ECC_Pawn);

	PhysicsHandle->GrabComponentAtLocationWithRotation(
		HitComp,
		NAME_None,
		HitComp->GetComponentLocation(),
		GetOwner()->GetActorRotation()
	);

	GrabbedRotation = HitComp->GetComponentRotation();

	HitComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	HitComp->SetUseCCD(true);

	// [수정] 중력 비활성화 + 강한 Linear/Angular 감쇠로 초기 요동 방지
	HitComp->SetEnableGravity(false);
	HitComp->SetLinearDamping(50.f);   // 추가: 선형 감쇠
	HitComp->SetAngularDamping(500.f);

	// [수정] 잡을 때 현재 속도를 0으로 초기화하여 잔여 운동량 제거
	HitComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
	HitComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

	PrevGrabLocation = HitComp->GetComponentLocation();
	bHasPrevLocation = true;
}

void UC_InteractionComponent::Push(UPrimitiveComponent* HitComp, const FHitResult& Hit)
{
	if (!PhysicsHandle || !HitComp) return;
	if (HitComp->GetMass() > PushMassLimit) return;

	CurrentState = EInteractionState::Pushing;
	bIsGrabbing = true;

	PushedComponent = HitComp;
	LockedRotation = GetOwner()->GetActorRotation();

	OriginalPawnResponse = HitComp->GetCollisionResponseToChannel(ECC_Pawn);

	// [수정] 밀기는 Block 유지 (캐릭터와 충돌해서 자연스럽게 밀림)
	HitComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	// [수정] 잡기 위치를 물체 중심으로 변경
	// 기존: Hit.ImpactPoint → 표면 위치라 불안정
	// 수정: 물체 중심에 잡아야 PhysicsHandle이 안정적으로 유지
	PhysicsHandle->GrabComponentAtLocationWithRotation(
		HitComp,
		NAME_None,
		HitComp->GetComponentLocation(),  // ImpactPoint → 중심으로 변경
		HitComp->GetComponentRotation()
	);

	// [수정] 밀기 물체 속도 초기화
	HitComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
	HitComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

	// [수정] 밀 때 물체가 옆으로 밀려나지 않도록 감쇠 증가
	HitComp->SetLinearDamping(5.f);
	HitComp->SetAngularDamping(100.f);

	// [수정] 중복 제거: 아래 ACharacter 직접 설정은 OnStartPush()와 동일한 작업을
	// 두 번 수행하던 것이라 OnStartPush()로 일원화함
	if (AEscapeGame260422Character* Char = Cast<AEscapeGame260422Character>(GetOwner()))
	{
		Char->OnStartPush();
	}
}


// ===================================================
// Release / Throw
// ===================================================

void UC_InteractionComponent::Release()
{
	if (PhysicsHandle && PhysicsHandle->GrabbedComponent)
	{
		UPrimitiveComponent* Comp = PhysicsHandle->GrabbedComponent;

		Comp->SetCollisionResponseToChannel(ECC_Pawn, OriginalPawnResponse);
		Comp->SetUseCCD(false);

		// [수정] 중력/감쇠 원래 값으로 복구
		Comp->SetEnableGravity(true);
		Comp->SetLinearDamping(0.01f);   // UE 기본값
		Comp->SetAngularDamping(0.1f);

		PhysicsHandle->ReleaseComponent();
	}

	// [수정] OnStopPush는 Push 상태에서 풀려날 때만 호출
	// Grab 해제 시 호출하면 카메라 암 길이가 불필요하게 리셋됨
	const bool bWasPushing = (CurrentState == EInteractionState::Pushing);

	// [수정] 중복 제거: ACharacter 직접 설정은 OnStopPush()가 동일하게 처리함
	bIsGrabbing = false;
	CurrentState = EInteractionState::None;
	bHasPrevLocation = false;

	if (bWasPushing)
	{
		if (AEscapeGame260422Character* Char = Cast<AEscapeGame260422Character>(GetOwner()))
		{
			Char->OnStopPush();
		}
	}
}

void UC_InteractionComponent::Throw()
{
	if (!PhysicsHandle || !PhysicsHandle->GrabbedComponent) return;

	UPrimitiveComponent* Comp = PhysicsHandle->GrabbedComponent;

	Release();

	// [수정] 던지기도 카메라 방향 기준으로 변경
	FVector ThrowDir;
	if (AEscapeGame260422Character* Char = Cast<AEscapeGame260422Character>(GetOwner()))
	{
		if (UCameraComponent* Cam = Char->GetFollowCamera())
		{
			ThrowDir = Cam->GetForwardVector();
		}
		else
		{
			FVector EyeLoc; FRotator EyeRot;
			GetOwner()->GetActorEyesViewPoint(EyeLoc, EyeRot);
			ThrowDir = EyeRot.Vector();
		}
	}
	else
	{
		FVector EyeLoc; FRotator EyeRot;
		GetOwner()->GetActorEyesViewPoint(EyeLoc, EyeRot);
		ThrowDir = EyeRot.Vector();
	}

	FVector Impulse = ThrowDir * ThrowForce;
	Impulse.Z += ThrowForce * 0.3f;

	Comp->AddImpulse(Impulse, NAME_None, true);
}

// ===================================================
// 데칼
// ===================================================

void UC_InteractionComponent::StartAiming()
{
	bIsAiming = true;
}

void UC_InteractionComponent::StopAiming()
{
	bIsAiming = false;
	if (LandingDecalComp)
		LandingDecalComp->SetVisibility(false);
}

void UC_InteractionComponent::UpdateLandingIndicator()
{
	if (!LandingDecalComp) return;

	FVector Loc, Normal;
	if (!PredictLanding(Loc, Normal))
	{
		LandingDecalComp->SetVisibility(false);
		return;
	}

	if (Normal.Z < 0.7f)
	{
		LandingDecalComp->SetVisibility(false);
		return;
	}

	LandingDecalComp->SetWorldLocation(Loc + Normal * 5.f);
	FRotator DecalRot = FRotationMatrix::MakeFromZ(-Normal).Rotator();
	LandingDecalComp->SetWorldRotation(DecalRot);
	LandingDecalComp->SetVisibility(true);
}

bool UC_InteractionComponent::PredictLanding(FVector& OutLoc, FVector& OutNormal)
{
	if (!PhysicsHandle || !PhysicsHandle->GrabbedComponent)
		return false;

	FVector Start = PhysicsHandle->GrabbedComponent->GetComponentLocation();

	// 카메라 방향 기준
	FVector ThrowDir;
	if (AEscapeGame260422Character* Char = Cast<AEscapeGame260422Character>(GetOwner()))
	{
		if (UCameraComponent* Cam = Char->GetFollowCamera())
			ThrowDir = Cam->GetForwardVector();
		else
		{
			FVector EyeLoc; FRotator EyeRot;
			GetOwner()->GetActorEyesViewPoint(EyeLoc, EyeRot);
			ThrowDir = EyeRot.Vector();
		}
	}
	else
	{
		FVector EyeLoc; FRotator EyeRot;
		GetOwner()->GetActorEyesViewPoint(EyeLoc, EyeRot);
		ThrowDir = EyeRot.Vector();
	}

	FVector Velocity = ThrowDir * ThrowForce;
	Velocity.Z += ThrowForce * 0.3f;

	FPredictProjectilePathParams Params;
	Params.StartLocation = Start;
	Params.LaunchVelocity = Velocity;
	Params.bTraceWithCollision = true;
	Params.ProjectileRadius = 5.f;
	Params.MaxSimTime = 3.f;
	Params.SimFrequency = 30.f;
	Params.TraceChannel = ECC_Visibility;
	Params.ActorsToIgnore.Add(GetOwner());

	FPredictProjectilePathResult Result;
	if (UGameplayStatics::PredictProjectilePath(this, Params, Result))
	{
		if (Result.HitResult.bBlockingHit)
		{
			OutLoc = Result.HitResult.ImpactPoint;
			OutNormal = Result.HitResult.ImpactNormal;
			return true;
		}
	}

	return false;
}