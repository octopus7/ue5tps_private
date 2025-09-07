#include "ThrowableGrenade.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

AThrowableGrenade::AThrowableGrenade()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    CollisionComp->InitSphereRadius(8.f);
    CollisionComp->SetCollisionProfileName(TEXT("PhysicsActor"));
    CollisionComp->SetSimulatePhysics(false); // movement by ProjectileMovement until stop
    CollisionComp->SetNotifyRigidBodyCollision(true);
    SetRootComponent(CollisionComp);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 1500.f;
    ProjectileMovement->MaxSpeed = 2000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = true;
    ProjectileMovement->Bounciness = 0.3f;
    ProjectileMovement->Friction = 0.2f;
    ProjectileMovement->ProjectileGravityScale = 1.0f;

    FuseTime = 2.5f;
    ExplosionDamage = 60.f;
    ExplosionRadius = 350.f;
    DamageTypeClass = UDamageType::StaticClass();
    PostExplosionLifespan = 2.0f;
    bExploded = false;
}

void AThrowableGrenade::BeginPlay()
{
    Super::BeginPlay();
}

void AThrowableGrenade::ActivateFuse()
{
    if (bExploded)
    {
        return;
    }
    // Start fuse from now; explode after FuseTime
    GetWorldTimerManager().SetTimer(TimerHandle_Fuse, this, &AThrowableGrenade::Explode, FMath::Max(0.01f, FuseTime), false);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Orange, FString::Printf(TEXT("Grenade fuse started: %.2fs"), FuseTime));
    }
    UE_LOG(LogTemp, Log, TEXT("Grenade(%s) fuse started: %.2fs"), *GetName(), FuseTime);
}

void AThrowableGrenade::Explode()
{
    if (bExploded)
    {
        return;
    }
    bExploded = true;

    // Spawn FX unattached so it can outlive this actor
    if (ExplosionEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), GetActorRotation());
    }

    // Apply radial damage
    AController* InstigatorController = nullptr;
    APawn* InstigatorPawn = Cast<APawn>(GetOwner());
    if (InstigatorPawn)
    {
        InstigatorController = InstigatorPawn->GetController();
    }
    UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, DamageTypeClass, TArray<AActor*>(), this, InstigatorController, true);

    // Debug: visualize damage radius as wireframe sphere for 0.5s
    DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 24, FColor::Red, false, 0.5f, 0, 1.5f);

    HideAndDisable();
    SetLifeSpan(PostExplosionLifespan);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Grenade exploded!"));
    }
    UE_LOG(LogTemp, Log, TEXT("Grenade(%s) exploded at %s radius=%.1f dmg=%.1f"), *GetName(), *GetActorLocation().ToString(), ExplosionRadius, ExplosionDamage);
}

void AThrowableGrenade::HideAndDisable()
{
    // Hide mesh and stop collisions/movement
    if (MeshComp)
    {
        MeshComp->SetVisibility(false, true);
    }
    if (CollisionComp)
    {
        CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (ProjectileMovement)
    {
        ProjectileMovement->StopMovementImmediately();
        ProjectileMovement->Deactivate();
    }
}
