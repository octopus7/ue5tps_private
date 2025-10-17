#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HealthPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * Collectible health item that heals overlapping actors immediately.
 */
UCLASS()
class HEAVENLYCASTLE_API AHealthPickup : public AActor
{
    GENERATED_BODY()

public:
    AHealthPickup();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    /** Amount of health restored on pickup. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "0.0"))
    float HealAmount;

    /** Radius used for overlap detection. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "0.0"))
    float CollisionRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "Pickup")
    float GetHealAmount() const { return HealAmount; }

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void SetHealAmount(float NewAmount) { HealAmount = FMath::Max(0.0f, NewAmount); }
};

