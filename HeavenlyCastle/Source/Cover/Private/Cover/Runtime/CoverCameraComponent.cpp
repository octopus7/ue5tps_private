#include "Cover/Runtime/CoverCameraComponent.h"

#include "Cover/Runtime/CoverControllerComponent.h"
#include "GameFramework/Actor.h"

UCoverCameraComponent::UCoverCameraComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    OffsetInterpSpeed = 10.f;
    CurrentOffset = FVector::ZeroVector;
}

void UCoverCameraComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* OwnerActor = GetOwner())
    {
        CachedController = OwnerActor->FindComponentByClass<UCoverControllerComponent>();
    }
}

void UCoverCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    FVector DesiredOffset = FVector::ZeroVector;

    UCoverControllerComponent* Controller = CachedController.Get();
    if (!Controller)
    {
        if (AActor* OwnerActor = GetOwner())
        {
            CachedController = OwnerActor->FindComponentByClass<UCoverControllerComponent>();
            Controller = CachedController.Get();
        }
    }

    if (Controller)
    {
        DesiredOffset = FVector(0.f, Controller->CurrentCameraXOffset, Controller->CurrentCameraZOffset);
    }

    CurrentOffset = FMath::VInterpTo(CurrentOffset, DesiredOffset, DeltaTime, OffsetInterpSpeed);

    OnCameraOffset.Broadcast(CurrentOffset);
}
