#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UHealthComponent*, HealthComponent, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEAVENLYCASTLE_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "0.0"))
    float MaxBodyHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float BodyHealth;

    void ApplyBodyHealthChange(float Delta, const class UDamageType* DamageType = nullptr, class AController* InstigatedBy = nullptr, AActor* DamageCauser = nullptr);

    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHealthChangedSignature OnHealthChanged;

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsDead() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetMaxHealth() const { return MaxBodyHealth; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool AddHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetDefaultHealth() const { return MaxBodyHealth; }
};
