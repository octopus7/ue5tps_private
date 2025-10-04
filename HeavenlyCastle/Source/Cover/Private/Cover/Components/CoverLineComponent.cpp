#include "Cover/Components/CoverLineComponent.h"

#include "Cover/Runtime/CoverRegistrySubsystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UCoverLineComponent::UCoverLineComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    Length = 300.f;
    Height = 110.f;
    Type = ECoverType::Low;
    bLeftOpen = true;
    bRightOpen = true;
    LocalNormal = FVector(0.f, -1.f, 0.f);
}

FCoverLine UCoverLineComponent::ToCoverLine() const
{
    FCoverLine Line;
    Line.Type = Type;
    Line.bLeftOpen = bLeftOpen;
    Line.bRightOpen = bRightOpen;
    Line.Height = Height;

    const FTransform WorldTransform = GetComponentTransform();
    const FVector WorldLocation = WorldTransform.GetLocation();
    const FVector RightVector = WorldTransform.GetUnitAxis(EAxis::Y);
    const FVector HalfOffset = RightVector * (Length * 0.5f);

    Line.Start = WorldLocation - HalfOffset;
    Line.End = WorldLocation + HalfOffset;
    Line.Normal = WorldTransform.TransformVectorNoScale(LocalNormal).GetSafeNormal();

    if (!Line.Normal.IsNearlyZero())
    {
        Line.Normal = Line.Normal.GetSafeNormal();
    }
    else
    {
        Line.Normal = FVector::CrossProduct((Line.End - Line.Start).GetSafeNormal(), FVector::UpVector).GetSafeNormal();
    }

    return Line;
}

void UCoverLineComponent::DebugDraw(UWorld* World, const FColor& LineColor) const
{
    if (!World)
    {
        return;
    }

    const FCoverLine Line = ToCoverLine();
    const FVector Center = (Line.Start + Line.End) * 0.5f;
    const FVector Tangent = (Line.End - Line.Start).GetSafeNormal();
    const FVector Normal = Line.Normal;

    DrawDebugLine(World, Line.Start, Line.End, LineColor, false, 0.f, 0, 2.f);

    // Normal tick marks along the line.
    const int32 Steps = 6;
    for (int32 Index = 0; Index <= Steps; ++Index)
    {
        const float Alpha = static_cast<float>(Index) / Steps;
        const FVector Point = FMath::Lerp(Line.Start, Line.End, Alpha);
        DrawDebugLine(World, Point, Point + Normal * 25.f, FColor::Cyan, false, 0.f, 0, 1.f);
    }

    // End caps showing open/closed state.
    const float CapSize = 18.f;
    const FVector LeftCapDir = Line.bLeftOpen ? Tangent : -Tangent;
    const FVector RightCapDir = Line.bRightOpen ? -Tangent : Tangent;

    DrawDebugLine(World, Line.Start, Line.Start + LeftCapDir * CapSize, Line.bLeftOpen ? FColor::Green : FColor::Red, false, 0.f, 0, 2.f);
    DrawDebugLine(World, Line.End, Line.End + RightCapDir * CapSize, Line.bRightOpen ? FColor::Green : FColor::Red, false, 0.f, 0, 2.f);

    DrawDebugString(World, Center + FVector(0.f, 0.f, 30.f),
        FString::Printf(TEXT("%s Cover\nH=%.0f"),
            Line.Type == ECoverType::Low ? TEXT("Low") : TEXT("High"),
            Line.Height),
        nullptr, FColor::White, 0.f, true);
}

void UCoverLineComponent::OnRegister()
{
    Super::OnRegister();
    RegisterWithSubsystem();
}

void UCoverLineComponent::OnUnregister()
{
    UnregisterFromSubsystem();
    Super::OnUnregister();
}

void UCoverLineComponent::DestroyComponent(bool bPromoteChildren)
{
    UnregisterFromSubsystem();
    Super::DestroyComponent(bPromoteChildren);
}

#if WITH_EDITOR
void UCoverLineComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    Length = FMath::Max(Length, 10.f);
    Height = FMath::Max(Height, 10.f);

    if (!LocalNormal.Normalize())
    {
        LocalNormal = FVector(0.f, -1.f, 0.f);
    }
}
#endif

void UCoverLineComponent::RegisterWithSubsystem()
{
    if (UWorld* World = GetWorld())
    {
        if (UCoverRegistrySubsystem* Registry = World->GetSubsystem<UCoverRegistrySubsystem>())
        {
            Registry->Register(this);
        }
    }
}

void UCoverLineComponent::UnregisterFromSubsystem()
{
    if (UWorld* World = GetWorld())
    {
        if (UCoverRegistrySubsystem* Registry = World->GetSubsystem<UCoverRegistrySubsystem>())
        {
            Registry->Unregister(this);
        }
    }
}
