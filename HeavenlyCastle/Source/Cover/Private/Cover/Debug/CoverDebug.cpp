#include "Cover/Debug/CoverDebug.h"

#include "Cover/Runtime/CoverControllerComponent.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

TAutoConsoleVariable<int32> CVarCoverDebug(
    TEXT("Cover.Debug"),
    0,
    TEXT("0=off,1=lines,2=verbose"),
    ECVF_Cheat);

void FCoverDebug::Draw(const UCoverControllerComponent& Controller)
{
    const int32 DebugMode = CVarCoverDebug.GetValueOnGameThread();
    if (DebugMode <= 0)
    {
        return;
    }

    UWorld* World = Controller.GetWorld();
    if (!World)
    {
        return;
    }

    if (Controller.State == ESimpleCoverState::Free || Controller.CurrentLineOwner == nullptr)
    {
        return;
    }

    const FCoverLine& Line = Controller.CurrentLine;
    const FVector Tangent = (Line.End - Line.Start).GetSafeNormal();
    const FVector Center = FMath::Lerp(Line.Start, Line.End, 0.5f);

    DrawDebugLine(World, Line.Start, Line.End, FColor::Green, false, -1.f, 0, 2.f);
    DrawDebugLine(World, Center, Center + Line.Normal * 40.f, FColor::Cyan, false, -1.f, 0, 1.f);

    if (DebugMode >= 2)
    {
        const float LineLength = (Line.End - Line.Start).Size();
        const float DistanceLeft = LineLength * Controller.LineT;
        const float DistanceRight = LineLength - DistanceLeft;

        const float RawLineT = Controller.GetRawLineT();
        const bool bFlipped = Controller.IsCoverSideFlipped();

        const FVector PointOnLine = FMath::Lerp(Line.Start, Line.End, RawLineT);
        DrawDebugSphere(World, PointOnLine, 6.f, 8, FColor::Yellow, false, -1.f);

        const FVector LeftEnd = bFlipped ? Line.End : Line.Start;
        const FVector RightEnd = bFlipped ? Line.Start : Line.End;

        DrawDebugBox(World, LeftEnd, FVector(2.f, 2.f, 40.f), FQuat::Identity, FColor::Magenta, false, -1.f, 0, 1.f);
        DrawDebugBox(World, RightEnd, FVector(2.f, 2.f, 40.f), FQuat::Identity, FColor::Magenta, false, -1.f, 0, 1.f);

        const FString StateString = StaticEnum<ESimpleCoverState>()->GetValueAsString(Controller.State);
        const FString DebugText = FString::Printf(TEXT("State: %s\nT: %.2f\nLeft %.0f / Right %.0f"), *StateString, Controller.LineT, DistanceLeft, DistanceRight);
        DrawDebugString(World, PointOnLine + FVector(0.f, 0.f, 50.f), DebugText, nullptr, FColor::White, 0.f, true);
    }
}
