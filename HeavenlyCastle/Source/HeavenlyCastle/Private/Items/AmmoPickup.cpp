#include "Items/AmmoPickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Weapon/AmmoConsumerInterface.h"

AAmmoPickup::AAmmoPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionRadius = 50.f;
    AmmoAmount = 30;
    bDestroyOnEmpty = true;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    CollisionComponent->InitSphereRadius(CollisionRadius);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollisionComponent->SetGenerateOverlapEvents(true);
    SetRootComponent(CollisionComponent);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetupAttachment(CollisionComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAmmoPickup::BeginPlay()
{
    Super::BeginPlay();

    CollisionComponent->SetSphereRadius(CollisionRadius);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AAmmoPickup::HandleOverlap);
}

void AAmmoPickup::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this || AmmoAmount <= 0)
    {
        return;
    }

    const int32 Used = TransferAmmoToActor(OtherActor);
    if (Used <= 0)
    {
        return;
    }

    AmmoAmount = FMath::Max(0, AmmoAmount - Used);

    if (AmmoAmount <= 0 && bDestroyOnEmpty)
    {
        Destroy();
    }
}

int32 AAmmoPickup::TransferAmmoToActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return 0;
    }

    if (!TargetActor->GetClass()->ImplementsInterface(UAmmoConsumerInterface::StaticClass()))
    {
        return 0;
    }

    return IAmmoConsumerInterface::Execute_AddAmmoToInventory(TargetActor, AmmoAmount);
}
