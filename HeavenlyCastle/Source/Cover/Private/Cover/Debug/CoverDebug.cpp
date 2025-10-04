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
        const bool bFlipped = Controller.IsCoverSideFlipped();

        const float RawT = Controller.LineT;
        const float DistanceToStart = LineLength * RawT;
        const float DistanceToEnd = LineLength - DistanceToStart;

        const float DistanceLeft = bFlipped ? DistanceToEnd : DistanceToStart;
        const float DistanceRight = bFlipped ? DistanceToStart : DistanceToEnd;

        const float DisplayT = bFlipped ? (1.f - RawT) : RawT;

        const FVector PointOnLine = FMath::Lerp(Line.Start, Line.End, RawT);
        DrawDebugSphere(World, PointOnLine, 6.f, 8, FColor::Yellow, false, -1.f);

        const FVector LeftEnd = bFlipped ? Line.End : Line.Start;
        const FVector RightEnd = bFlipped ? Line.Start : Line.End;
        const bool bLeftOpen = bFlipped ? Line.bRightOpen : Line.bLeftOpen;
        const bool bRightOpen = bFlipped ? Line.bLeftOpen : Line.bRightOpen;

        DrawDebugBox(World, LeftEnd, FVector(2.f, 2.f, 40.f), FQuat::Identity, FColor::Magenta, false, -1.f, 0, 1.f);
        DrawDebugBox(World, RightEnd, FVector(2.f, 2.f, 40.f), FQuat::Identity, FColor::Magenta, false, -1.f, 0, 1.f);

        const FVector LabelOffset(0.f, 0.f, 35.f);
        DrawDebugString(World, LeftEnd + LabelOffset, bLeftOpen ? TEXT("Left=") : TEXT("Left=#"), nullptr, bLeftOpen ? FColor::Green : FColor::Red, 0.f, true);
        DrawDebugString(World, RightEnd + LabelOffset, bRightOpen ? TEXT("Right=") : TEXT("Right=#"), nullptr, bRightOpen ? FColor::Green : FColor::Red, 0.f, true);

        const FString StateString = StaticEnum<ESimpleCoverState>()->GetValueAsString(Controller.State);
        const FString DebugText = FString::Printf(TEXT("State: %s\nT: %.2f\nLeft %.0f / Right %.0f"), *StateString, DisplayT, DistanceLeft, DistanceRight);
        DrawDebugString(World, PointOnLine + FVector(0.f, 0.f, 50.f), DebugText, nullptr, FColor::White, 0.f, true);
    }
}
