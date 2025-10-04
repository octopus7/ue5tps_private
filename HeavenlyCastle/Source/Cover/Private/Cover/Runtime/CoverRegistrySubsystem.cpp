#include "Cover/Runtime/CoverRegistrySubsystem.h"

#include "Cover/Components/CoverLineComponent.h"
#include "Cover/Debug/CoverDebug.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogCoverRegistry, Log, All);

void UCoverRegistrySubsystem::Register(UCoverLineComponent* LineComponent)
{
    if (!IsValid(LineComponent))
    {
        return;
    }

    RegisteredLines.AddUnique(LineComponent);

    UE_LOG(LogCoverRegistry, Log, TEXT("Register cover line: %s"), *GetNameSafe(LineComponent));

    const int32 DebugMode = CVarCoverDebug.GetValueOnGameThread();
    if (DebugMode >= 2 && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
            FString::Printf(TEXT("CoverRegistry: Register %s"), *GetNameSafe(LineComponent)));
    }
}

void UCoverRegistrySubsystem::Unregister(UCoverLineComponent* LineComponent)
{
    if (!LineComponent)
    {
        return;
    }

    RegisteredLines.Remove(LineComponent);

    UE_LOG(LogCoverRegistry, Log, TEXT("Unregister cover line: %s"), *GetNameSafe(LineComponent));

    const int32 DebugMode = CVarCoverDebug.GetValueOnGameThread();
    if (DebugMode >= 2 && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
            FString::Printf(TEXT("CoverRegistry: Unregister %s"), *GetNameSafe(LineComponent)));
    }
}

bool UCoverRegistrySubsystem::FindBestCoverLine(const FVector& From, const FVector& Forward, float MaxDist, float MaxAngleDeg,
    FCoverLine& OutLine, UCoverLineComponent*& OutOwner) const
{
    const float MaxDistSq = FMath::Square(MaxDist);
    const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(MaxAngleDeg));
    const FVector ForwardNorm = Forward.IsNearlyZero() ? FVector::XAxisVector : Forward.GetSafeNormal();

    bool bFound = false;
    float BestScore = MAX_flt;
    FCoverLine BestLine;
    UCoverLineComponent* BestOwner = nullptr;

    for (int32 Index = 0; Index < RegisteredLines.Num(); ++Index)
    {
        UCoverLineComponent* LineComponent = RegisteredLines[Index].Get();
        if (!IsValid(LineComponent) || !LineComponent->IsRegistered())
        {
            continue;
        }

        const FCoverLine Line = LineComponent->ToCoverLine();

        float T = 0.f;
        const FVector ClosestPoint = FCoverLine::ClosestPointOnSegment(From, Line.Start, Line.End, &T);
        const float DistSq = FVector::DistSquared(From, ClosestPoint);
        if (DistSq > MaxDistSq)
        {
            continue;
        }

        const FVector LineNormal = Line.Normal.IsNearlyZero() ? FVector::ForwardVector : Line.Normal.GetSafeNormal();
        const float FacingDot = FVector::DotProduct(ForwardNorm, -LineNormal);
        if (FacingDot < CosThreshold)
        {
            continue;
        }

        const FVector ToPoint = ClosestPoint - From;
        const FVector ToPointDir = ToPoint.IsNearlyZero() ? -LineNormal : ToPoint.GetSafeNormal();
        const float ApproachDot = FVector::DotProduct(ToPointDir, -LineNormal);

        const float DistanceTerm = DistSq;
        const float FacingTerm = 1.f - FacingDot; // 0이면 완전 정렬
        const float ApproachTerm = 1.f - ApproachDot;

        const float Score = DistanceTerm + FacingTerm * 150.f + ApproachTerm * 75.f;
        if (Score < BestScore)
        {
            BestScore = Score;
            BestLine = Line;
            BestOwner = LineComponent;
            bFound = true;
        }
    }

    if (bFound)
    {
        OutLine = BestLine;
        OutOwner = BestOwner;
    }

    return bFound;
}

void UCoverRegistrySubsystem::DrawDebug(UWorld* World, int32 DebugMode) const
{
    if (!World || DebugMode <= 0)
    {
        return;
    }

    for (const TWeakObjectPtr<UCoverLineComponent>& WeakLineComponent : RegisteredLines)
    {
        UCoverLineComponent* LineComponent = WeakLineComponent.Get();
        if (!IsValid(LineComponent) || !LineComponent->IsRegistered())
        {
            continue;
        }

        const FCoverLine Line = LineComponent->ToCoverLine();
        const FColor BaseColor = (Line.Type == ECoverType::High) ? FColor::Orange : FColor::Green;
        LineComponent->DebugDraw(World, BaseColor);

        if (DebugMode >= 2)
        {
            const FVector LabelLocation = (Line.Start + Line.End) * 0.5f + FVector(0.f, 0.f, Line.Height + 20.f);
            const FString Label = FString::Printf(TEXT("%s\nL=%.0f"), *LineComponent->GetName(), (Line.End - Line.Start).Size());
            DrawDebugString(World, LabelLocation, Label, nullptr, FColor::White, 0.f, true);
        }
    }
}
