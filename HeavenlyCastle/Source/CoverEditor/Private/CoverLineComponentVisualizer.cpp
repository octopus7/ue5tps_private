#include "CoverLineComponentVisualizer.h"

#include "Cover/Components/CoverLineComponent.h"
#include "SceneManagement.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

void FCoverLineComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    const UCoverLineComponent* CoverComp = Cast<const UCoverLineComponent>(Component);
    if (!CoverComp)
    {
        return;
    }

    const FCoverLine Line = CoverComp->ToCoverLine();
    const FVector Segment = Line.End - Line.Start;
    const float SegmentLength = Segment.Size();
    if (SegmentLength <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const FVector Center = (Line.Start + Line.End) * 0.5f;
    const FVector Tangent = Segment / SegmentLength;

    FVector BoxUp = FVector::UpVector;
    if (FMath::Abs(FVector::DotProduct(BoxUp, Tangent)) > 0.99f)
    {
        BoxUp = FVector::RightVector;
    }

    FVector BoxBinormal = FVector::CrossProduct(BoxUp, Tangent).GetSafeNormal();
    if (BoxBinormal.IsNearlyZero())
    {
        BoxBinormal = FVector::CrossProduct(Tangent, FVector::ForwardVector).GetSafeNormal();
    }

    FVector BoxNormal = FVector::CrossProduct(Tangent, BoxBinormal).GetSafeNormal();
    if (BoxNormal.IsNearlyZero())
    {
        BoxNormal = FVector::UpVector;
    }

    // Compute the displayed normal arrow using the actual cover normal projected onto the line plane.
    FVector ArrowNormal = Line.Normal;
    if (!ArrowNormal.Normalize())
    {
        ArrowNormal = BoxNormal;
    }

    ArrowNormal -= Tangent * FVector::DotProduct(ArrowNormal, Tangent);
    if (!ArrowNormal.Normalize())
    {
        ArrowNormal = BoxNormal;
    }

    if (!Line.Normal.IsNearlyZero() && FVector::DotProduct(ArrowNormal, Line.Normal) < 0.f)
    {
        ArrowNormal *= -1.f;
    }

    const float HalfLength = SegmentLength * 0.5f;
    const float HalfThickness = 15.f;
    const float HalfHeight = FMath::Max(Line.Height * 0.5f, 10.f);

    const FVector Extents(HalfLength, HalfThickness, HalfHeight);
    const FLinearColor BoxColor = (Line.Type == ECoverType::High) ? FLinearColor(1.f, 0.35f, 0.f) : FLinearColor(0.f, 0.8f, 0.f);
    DrawOrientedWireBox(PDI, Center, Tangent, BoxBinormal, BoxNormal, Extents, BoxColor, SDPG_Foreground);

    const FVector ArrowBase = Center - BoxNormal * HalfHeight;
    const FVector ArrowEnd = ArrowBase + ArrowNormal * 80.f;
    PDI->DrawLine(ArrowBase, ArrowEnd, FLinearColor(0.f, 0.75f, 1.f, 1.f), SDPG_Foreground, 2.f);

    if (GEngine)
    {
        const FVector TextLocation = ArrowBase + ArrowNormal * 40.f + BoxBinormal * 5.f;
        if (const UWorld* World = Component->GetWorld())
        {
            const FColor TextColor = FLinearColor(0.f, 0.75f, 1.f, 1.f).ToFColor(true);
            DrawDebugString(World, TextLocation, TEXT("Entry Direction"), nullptr, TextColor, 0.f, true, 1.f);
        }
    }
}
