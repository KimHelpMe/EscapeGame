// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeGame260422Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AEscapeGame260422Character::AEscapeGame260422Character()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CurrentStamina = MaxStamina;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	InteractionComp = CreateDefaultSubobject<UC_InteractionComponent>(TEXT("InteractionComponent"));
}

void AEscapeGame260422Character::BeginPlay()
{
	Super::BeginPlay();

	DefaultArmLength = CameraBoom->TargetArmLength;

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// [수정] BeginPlay 순서 문제 해결
	// InteractHintActor::BeginPlay가 아직 실행 안 된 상태에서 InitializeAllHintActors를 호출하면
	// 서브시스템 등록 전에 숨겨버려서 힌트가 영원히 안 보임
	// → 한 프레임 뒤에 실행해서 모든 액터 BeginPlay가 끝난 뒤 초기화
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AEscapeGame260422Character::InitializeAllHintActors);
}

void AEscapeGame260422Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEscapeGame260422Character::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEscapeGame260422Character::Look);

		EnhancedInputComponent->BindAction(IA_ViewToggle, ETriggerEvent::Started, this, &AEscapeGame260422Character::TogglePuzzleHighlight);

		if (IA_Grab)
		{
			EnhancedInputComponent->BindAction(IA_Grab, ETriggerEvent::Started, InteractionComp, &UC_InteractionComponent::TryInteract);
			EnhancedInputComponent->BindAction(IA_Grab, ETriggerEvent::Completed, InteractionComp, &UC_InteractionComponent::Release);
		}

		if (IA_Throw)
		{
			EnhancedInputComponent->BindAction(IA_Throw, ETriggerEvent::Started, this, &AEscapeGame260422Character::StartAim);
			EnhancedInputComponent->BindAction(IA_Throw, ETriggerEvent::Completed, this, &AEscapeGame260422Character::ThrowObject);
		}

		if (IA_Sprint)
		{
			EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AEscapeGame260422Character::StartSprint);
			EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Completed, this, & AEscapeGame260422Character::StopSprint);
		}

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AEscapeGame260422Character::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (InteractionComp && InteractionComp->IsPushing())
	{
		if (Controller != nullptr && MovementVector.Y > 0.f)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(ForwardDirection, MovementVector.Y * 0.3f);
		}
		return;
	}

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AEscapeGame260422Character::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AEscapeGame260422Character::ThrowObject()
{
	if (InteractionComp)
	{
		InteractionComp->Throw();
		InteractionComp->StopAiming();
	}
}

void AEscapeGame260422Character::StartAim()
{
	if (InteractionComp) InteractionComp->StartAiming();
}

void AEscapeGame260422Character::StopAim()
{
	if (InteractionComp) InteractionComp->StopAiming();
}

bool AEscapeGame260422Character::IsPushing() const
{
	return InteractionComp && InteractionComp->IsPushing();
}

void AEscapeGame260422Character::OnStartPush()
{
	GetCharacterMovement()->MaxWalkSpeed = 150.f;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	CameraBoom->TargetArmLength = 250.f;
}

void AEscapeGame260422Character::OnStopPush()
{
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	CameraBoom->TargetArmLength = DefaultArmLength;
}

void AEscapeGame260422Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Sprint & Stamina 관련
	if (bIsSprinting && GetVelocity().Size() > 0.f)
	{
		CurrentStamina = FMath::Clamp(CurrentStamina - StaminaDrainRate * DeltaTime, 0.f, MaxStamina);

		if (CurrentStamina <= 0.f)
		{
			// 스태미나 전부 소진 시 탈진
			bIsSprinting = false;
			bIsExhausted = true;
			GetCharacterMovement()->MaxWalkSpeed = ExhaustedSpeed;

		}
	}
	else
	{
		//스태미나 회복

		float RegenRate = StaminaRegenRate;
		if (GetVelocity().Size() < 1.f)
		{
			RegenRate *= StaminaIdleRegenMultiplier;
		}

		CurrentStamina = FMath::Clamp(CurrentStamina + RegenRate * DeltaTime, 0.f, MaxStamina);

		//탈진 해제 후 회복
		if (bIsExhausted && GetStaminaPercent() >= ExhaustionRecoverThreshold)
		{
			bIsExhausted = false;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}

	}



}

void AEscapeGame260422Character::InitializeAllHintActors()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractHintActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (AInteractHintActor* Hint = Cast<AInteractHintActor>(Actor))
		{
			Hint->ResetHintState();
			Hint->SetPuzzleActive(false);
		}
	}
}

void AEscapeGame260422Character::TogglePuzzleMode()
{
	bPuzzleMode = !bPuzzleMode;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPuzzleHighlightSubsystem* Subsys = GI->GetSubsystem<UPuzzleHighlightSubsystem>())
		{
			Subsys->SetHighlightEnabled(bPuzzleMode);
		}
	}

	// 서브시스템 미등록 액터 백업 처리
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractHintActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (AInteractHintActor* Hint = Cast<AInteractHintActor>(Actor))
		{
			if (bPuzzleMode)
				Hint->SetPuzzleActive(true);
			else
			{
				Hint->ResetHintState();
				Hint->SetPuzzleActive(false);
			}
		}
	}
}

void AEscapeGame260422Character::SetFirstPerson(bool bEnable)
{
	bIsFirstPerson = bEnable;
}

void AEscapeGame260422Character::TogglePuzzleHighlight()
{
	TogglePuzzleMode();
}

void AEscapeGame260422Character::UpdatePuzzleHighlightState()
{
	// TogglePuzzleMode 내부에서 처리됨
}


// ===== Stamina =====

void AEscapeGame260422Character::StartSprint(const FInputActionValue& Value)
{
	if (!bIsExhausted && CurrentStamina > 0.f)
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

	}
}

void AEscapeGame260422Character::StopSprint(const FInputActionValue& Value)
{
	bIsSprinting = false;
	if (!bIsExhausted)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

float AEscapeGame260422Character::GetStaminaPercent() const
{
	return MaxStamina > 0.f ? CurrentStamina / MaxStamina : 0.f;
}