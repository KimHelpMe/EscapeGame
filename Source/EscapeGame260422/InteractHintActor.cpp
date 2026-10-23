#include "InteractHintActor.h"
#include "Components/StaticMeshComponent.h"
#include "PuzzleHighlightSubsystem.h"
#include "Kismet/GameplayStatics.h"

AInteractHintActor::AInteractHintActor()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// [수정] 시작부터 숨김 상태
	Mesh->SetRenderCustomDepth(false);
	SetActorHiddenInGame(true);
}

void AInteractHintActor::BeginPlay()
{
	Super::BeginPlay();

	FloatOrigin = GetActorLocation();

	// 그랩 가능한 힌트는 물리 먼저 활성화
	if (bGrabbable && Mesh)
	{
		Mesh->SetSimulatePhysics(true);
		//Mesh->SetEnableGravity(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// 물리 활성화 후 숨김 처리
	SetActorHiddenInGame(true);
	if (Mesh)
	{
		Mesh->SetVisibility(false);
		Mesh->SetRenderCustomDepth(false);
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UPuzzleHighlightSubsystem* Subsys = GI->GetSubsystem<UPuzzleHighlightSubsystem>())
		{
			Subsys->RegisterActor(this);
		}
	}
}

void AInteractHintActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFirstPersonVisible && !bGrabbable && !FloatAmplitude.IsZero())
	{
		FloatTime += DeltaTime;
		FVector Offset = FloatAmplitude * FMath::Sin(FloatTime * FloatFrequency * PI * 2.f);
		SetActorLocation(FloatOrigin + Offset);
	}

}

// =====================================================
// [수정] SetPuzzleActive 재설계
//
// 기존: bFirstPersonVisible 상태와 혼재되어 혼란
// 수정: 두 단계로 명확히 분리
//   - Active(true)  : 오브젝트 보임, 아직 glow 없음 (상호작용 대기)
//   - Active(false) : 완전히 숨김 + 상태 초기화
// =====================================================
void AInteractHintActor::SetPuzzleActive(bool bActive)
{
	bFirstPersonVisible = bActive;

	if (!Mesh) return;

	if (bActive)
	{
		SetActorHiddenInGame(false);
		Mesh->SetVisibility(true);

		// 블루프린트에서 추가한 SpotLight 포함 모든 라이트 컴포넌트 활성화
		TArray<ULightComponent*> Lights;
		GetComponents<ULightComponent>(Lights);
		for (ULightComponent* Light : Lights)
		{
			Light->SetVisibility(true);
		}
	}
	else
	{
		SetActorHiddenInGame(true);
		Mesh->SetVisibility(false);
		Mesh->SetRenderCustomDepth(false);

		// 블루프린트에서 추가한 SpotLight 포함 모든 라이트 컴포넌트 비활성화
		TArray<ULightComponent*> Lights;
		GetComponents<ULightComponent>(Lights);
		for (ULightComponent* Light : Lights)
		{
			Light->SetVisibility(false);
		}

		bActivated = false;
		bInteractable = false;
		FocusTime = 0.f;
	}
}

// =====================================================
// [수정] SetFirstPersonVisible - SetPuzzleActive와 분리
// 1인칭 카메라 전용 가시성 (현재 시스템에선 사용 안 함)
// =====================================================
void AInteractHintActor::SetFirstPersonVisible(bool bVisible)
{
	bFirstPersonVisible = bVisible;
	SetActorHiddenInGame(!bVisible);
	if (Mesh)
	{
		Mesh->SetVisibility(bVisible);
	}
}

// 시선 집중 시간 누적 → 일정 시간 지나면 ActivateHint 호출
void AInteractHintActor::UpdateFocus(bool bInFocused, float DeltaTime)
{
	if (!bFirstPersonVisible)
		return;

	if (bInFocused)
	{
		FocusTime += DeltaTime;
		if (FocusTime >= RequiredFocusTime)
		{
			ActivateHint();
		}
	}
	else
	{
		// [수정] 즉시 0으로 리셋하지 않고 FocusDecayRate에 따라 천천히 감소
		// 사용자가 살짝 시선 벗어나도 다시 응시하면 누적 유지됨
		FocusTime = FMath::Max(0.f, FocusTime - FocusDecayRate * DeltaTime);

		// 활성화 임계값보다 낮으면 interactable 해제
		if (FocusTime < RequiredFocusTime)
		{
			bInteractable = false;
		}
	}

	// [추가] 응시 비율을 서브시스템에 전달
	// PostProcess Material이 Material Parameter Collection을 통해 이 값을 읽음
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPuzzleHighlightSubsystem* Subsys = GI->GetSubsystem<UPuzzleHighlightSubsystem>())
		{
			Subsys->SetFocusRatio(GetFocusRatio());
		}
	}
}

// =====================================================
// ActivateHint: glow 활성화 + interactable 상태로 전환
// V키로 오브젝트가 보이는 상태에서 상호작용 시 호출
// =====================================================
void AInteractHintActor::ActivateHint()
{
	UE_LOG(LogTemp, Warning, TEXT("[HintActor] ActivateHint 호출됨"));
	if (bActivated)
		return;

	bActivated = true;
	bInteractable = true;

	if (Mesh)
	{
		// CustomDepth glow 활성화 (외곽선용 - PostProcess Material에서 처리)
		Mesh->SetRenderCustomDepth(true);
		Mesh->CustomDepthStencilValue = 7;

		// 내부 글로우 머티리얼로 교체 (보이는 타입만 효과 있음, 투명 타입은 같은 머티리얼이라 변화 없음)
		if (MaterialGlow)
		{
			Mesh->SetMaterial(0, MaterialGlow);
		}
	}
}

void AInteractHintActor::ResetHintState()
{
	bActivated = false;
	bInteractable = false;
	FocusTime = 0.f;

	if (Mesh)
	{
		Mesh->SetRenderCustomDepth(false);

		// 기본 머티리얼로 복원 (보이는 타입은 평상시 머티리얼로, 투명 타입은 같은 머티리얼이라 변화 없음)
		if (MaterialBase)
		{
			Mesh->SetMaterial(0, MaterialBase);
		}
	}
}
