#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThrowableGrenade.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class HEAVENLYCASTLE_API AThrowableGrenade : public AActor
{
    GENERATED_BODY()

public:
    AThrowableGrenade();

    // Activate the fuse when thrown; explosion happens after FuseTime seconds
    UFUNCTION(BlueprintCallable, Category = "Grenade")
    void ActivateFuse();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void Explode();

    // After explode, hide mesh and stop collision
    void HideAndDisable();

public:
    // Components
    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    USphereComponent* CollisionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProjectileMovementComponent* ProjectileMovement;

    // Fuse time after throw
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
    float FuseTime;

    // Radial damage on explode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
    float ExplosionDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
    float ExplosionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
    TSubclassOf<UDamageType> DamageTypeClass;

    // Niagara effect to spawn on explosion (spawned unattached)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade|Effects")
    UNiagaraSystem* ExplosionEffect;

    // Optional: linger lifespan after explosion (mesh hidden)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
    float PostExplosionLifespan;

private:
    bool bExploded;
    FTimerHandle TimerHandle_Fuse;
};
