#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaxBodyHealth = 100.0f;
    BodyHealth = MaxBodyHealth;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    BodyHealth = MaxBodyHealth;

    AActor* MyOwner = GetOwner();
    if (MyOwner)
    {
        MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
    }
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f || IsDead())
    {
        return;
    }

    ApplyBodyHealthChange(-Damage, DamageType, InstigatedBy, DamageCauser);
}

void UHealthComponent::ApplyBodyHealthChange(float Delta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (FMath::IsNearlyZero(Delta))
    {
        return;
    }

    const float OldHealth = BodyHealth;
    BodyHealth = FMath::Clamp(BodyHealth + Delta, 0.0f, MaxBodyHealth);
    const float ActualDelta = BodyHealth - OldHealth;

    if (!FMath::IsNearlyZero(ActualDelta))
    {
        OnHealthChanged.Broadcast(this, BodyHealth, ActualDelta, DamageType, InstigatedBy, DamageCauser);
    }
}

bool UHealthComponent::AddHealth(float Amount)
{
    if (Amount <= 0.0f || IsDead())
    {
        return false;
    }

    const float PreviousHealth = BodyHealth;
    ApplyBodyHealthChange(Amount);
    return !FMath::IsNearlyEqual(PreviousHealth, BodyHealth);
}

float UHealthComponent::GetHealth() const
{
    return BodyHealth;
}

bool UHealthComponent::IsDead() const
{
    return BodyHealth <= 0.0f;
}

float UHealthComponent::GetHealthPercent() const
{
    if (MaxBodyHealth <= 0.0f)
    {
        return 0.0f;
    }

    return BodyHealth / MaxBodyHealth;
}
