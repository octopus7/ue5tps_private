// Copyright Epic Games, Inc. All Rights Reserved.

#include "SummerTPSProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASummerTPSProjectile::ASummerTPSProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ASummerTPSProjectile::OnHit);		// set up a notification for when this component hits something blocking
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ASummerTPSProjectile::OnOverlapBegin);

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

	Damage = 100.0f;
}

// Called when the game starts or when spawned
void ASummerTPSProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SpawnEffect, GetActorLocation());
	}
}

// Called every frame
void ASummerTPSProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASummerTPSProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AActor* MyOwner = GetOwner();

	// Ignore collision with self, the owner (player), and any other actors the owner owns (like the weapon).
	if (OtherActor && (OtherActor == this || OtherActor == MyOwner || OtherActor->GetOwner() == MyOwner))
	{
		return;
	}

	UGameplayStatics::ApplyDamage(OtherActor, Damage, MyOwner->GetInstigatorController(), this, UDamageType::StaticClass());

	// If we hit anything else (including world geometry where OtherActor is null), spawn the effect.
	if (OverlapEffect)
	{
		// Use the impact point from the sweep result for a more accurate spawn location.
		// Fall back to the actor's transform if the impact point is not available.
		const FTransform SpawnTransform = bFromSweep ? FTransform(SweepResult.ImpactNormal.Rotation(), SweepResult.ImpactPoint) : GetActorTransform();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), OverlapEffect, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator());
	}

	// Destroy the projectile after the effect has been spawned.
	Destroy();
}

void ASummerTPSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AActor* MyOwner = GetOwner();

	// Ignore collision with self, the owner (player), and any other actors the owner owns (like the weapon).
	if (OtherActor && (OtherActor == this || OtherActor == MyOwner || OtherActor->GetOwner() == MyOwner))
	{
		return;
	}

	UGameplayStatics::ApplyDamage(OtherActor, Damage, MyOwner->GetInstigatorController(), this, UDamageType::StaticClass());

	// If we hit anything else, spawn the effect at the impact point.
	if (OverlapEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), OverlapEffect, Hit.ImpactPoint);
	}

	// Destroy the projectile after the effect has been spawned.
	Destroy();
}
