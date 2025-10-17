#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * Collectible ammo that transfers spare rounds to actors implementing UAmmoConsumerInterface.
 */
UCLASS()
class HEAVENLYCASTLE_API AAmmoPickup : public AActor
{
    GENERATED_BODY()

public:
    AAmmoPickup();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    int32 TransferAmmoToActor(AActor* TargetActor);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    /** Current rounds stored in this pickup. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "0"))
    int32 AmmoAmount;

    /** Radius used for overlap detection. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "0.0"))
    float CollisionRadius;

    /** If true, destroy the actor when the stored ammo is depleted. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
    bool bDestroyOnEmpty;

public:
    UFUNCTION(BlueprintCallable, Category = "Pickup")
    int32 GetAmmoAmount() const { return AmmoAmount; }

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void SetAmmoAmount(int32 NewAmount) { AmmoAmount = FMath::Max(0, NewAmount); }
};
