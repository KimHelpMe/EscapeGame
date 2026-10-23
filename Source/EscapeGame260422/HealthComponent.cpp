#include "HealthComponent.h"


UHealthComponent::UHealthComponent()
{
	MaxHP = 100.0f;
	CurrentHP = MaxHP;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
}



void UHealthComponent::TakeDamage(float DamageAmount)
{
	if (IsDead()) return;

	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);

	if (CurrentHP <= 0.0f)
	{
		OnDeath.Broadcast();
	}

}


bool UHealthComponent::IsDead() const
{
	return CurrentHP <= 0.0f;
}

