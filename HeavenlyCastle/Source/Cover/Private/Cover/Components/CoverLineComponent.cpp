#include "Cover/Components/CoverLineComponent.h"

#include "Cover/Runtime/CoverRegistrySubsystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Math/RotationMatrix.h"

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
    const FVector Tangent = WorldTransform.GetUnitAxis(EAxis::X);
    const FVector HalfOffset = Tangent * (Length * 0.5f);

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
    const FColor EffectiveColor = (Line.Type == ECoverType::High) ? FColor::Orange : LineColor;
    const FVector SegmentVector = Line.End - Line.Start;
    const FVector Center = (Line.Start + Line.End) * 0.5f;
    const FVector Tangent = SegmentVector.GetSafeNormal();
    const FVector Normal = Line.Normal;

    DrawDebugLine(World, Line.Start, Line.End, EffectiveColor, false, 0.f, 0, 2.f);

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
    FVector SegmentDir = SegmentVector;
    if (!SegmentDir.Normalize())
    {
        SegmentDir = FVector::RightVector;
    }

    FVector Forward = Line.Normal;
    if (!Forward.Normalize())
    {
        Forward = FVector::ForwardVector;
    }

    FVector Up = FVector::UpVector;
    if (FMath::Abs(FVector::DotProduct(Forward, Up)) > 0.99f)
    {
        Up = FVector::RightVector;
    }

    FVector Right = FVector::CrossProduct(Forward, Up).GetSafeNormal();
    if (Right.IsNearlyZero())
    {
        Right = FVector::CrossProduct(SegmentDir, Up).GetSafeNormal();
    }

    if (Right.IsNearlyZero())
    {
        Right = FVector::RightVector;
    }

    const FVector LeftDir = -Right;
    const bool bLeftIsEnd = FVector::DotProduct(SegmentDir, LeftDir) > 0.f;

    const FVector LeftPoint = bLeftIsEnd ? Line.End : Line.Start;
    const FVector RightPoint = bLeftIsEnd ? Line.Start : Line.End;
    const bool bVisualLeftOpen = bLeftIsEnd ? Line.bRightOpen : Line.bLeftOpen;
    const bool bVisualRightOpen = bLeftIsEnd ? Line.bLeftOpen : Line.bRightOpen;

    const FVector LeftAlongSegment = bLeftIsEnd ? -SegmentDir : SegmentDir;
    const FVector RightAlongSegment = -LeftAlongSegment;

    const FVector LeftCapDir = bVisualLeftOpen ? LeftAlongSegment : -LeftAlongSegment;
    const FVector RightCapDir = bVisualRightOpen ? RightAlongSegment : -RightAlongSegment;

    DrawDebugLine(World, LeftPoint, LeftPoint + LeftCapDir * CapSize, bVisualLeftOpen ? FColor::Green : FColor::Red, false, 0.f, 0, 2.f);
    DrawDebugLine(World, RightPoint, RightPoint + RightCapDir * CapSize, bVisualRightOpen ? FColor::Green : FColor::Red, false, 0.f, 0, 2.f);

    const FVector LeftLabelLocation = LeftPoint + FVector(0.f, 0.f, 35.f);
    const FVector RightLabelLocation = RightPoint + FVector(0.f, 0.f, 35.f);
    DrawDebugString(World, LeftLabelLocation, bVisualLeftOpen ? TEXT("Left=") : TEXT("Left=#"), nullptr, bVisualLeftOpen ? FColor::Green : FColor::Red, 0.f, true);
    DrawDebugString(World, RightLabelLocation, bVisualRightOpen ? TEXT("Right=") : TEXT("Right=#"), nullptr, bVisualRightOpen ? FColor::Green : FColor::Red, 0.f, true);

    DrawDebugString(World, Center + FVector(0.f, 0.f, 30.f),
        FString::Printf(TEXT("%s Cover\nH=%.0f"),
            Line.Type == ECoverType::Low ? TEXT("Low") : TEXT("High"),
            Line.Height),
        nullptr, FColor::White, 0.f, true);

    const float SegmentLength = SegmentVector.Size();
    if (SegmentLength > KINDA_SMALL_NUMBER)
    {
        FVector UpVector = FVector::UpVector;
        if (FMath::Abs(FVector::DotProduct(UpVector, SegmentVector.GetSafeNormal())) > 0.95f)
        {
            UpVector = FVector::RightVector;
        }

        const FVector XAxis = SegmentVector.GetSafeNormal();
        FVector YAxis = FVector::CrossProduct(UpVector, XAxis).GetSafeNormal();
        FVector ZAxis = FVector::CrossProduct(XAxis, YAxis).GetSafeNormal();

        if (!Line.Normal.IsNearlyZero() && FVector::DotProduct(YAxis, Line.Normal.GetSafeNormal()) < 0.f)
        {
            YAxis *= -1.f;
            ZAxis *= -1.f;
        }

        const float HalfLength = SegmentLength * 0.5f;
        const float HalfThickness = 15.f;
        const float HalfHeight = FMath::Max(Line.Height * 0.5f, 10.f);
        const FVector BoxExtents(HalfLength, HalfThickness, HalfHeight);
        const FVector BoxCenter = Center + FVector(0.f, 0.f, HalfHeight);

        const FMatrix RotationMatrix = FRotationMatrix::MakeFromXY(XAxis, YAxis);
        const FQuat BoxRotation = RotationMatrix.ToQuat();

        DrawDebugBox(World, BoxCenter, BoxExtents, BoxRotation, EffectiveColor, false, 0.f, 0, 1.5f);
    }
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
