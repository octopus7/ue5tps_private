#include "HealthComponent.h"
#include "GameFramework/Actor.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    DefaultHealth = 100.0f;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    Health = DefaultHealth;

    AActor* MyOwner = GetOwner();
    if (MyOwner)
    {
        MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
    }
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f || IsDead())
    {
        return;
    }

    Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

    OnHealthChanged.Broadcast(this, Health, -Damage, DamageType, InstigatedBy, DamageCauser);
}

float UHealthComponent::GetHealth() const
{
    return Health;
}

bool UHealthComponent::IsDead() const
{
    return Health <= 0.0f;
}
