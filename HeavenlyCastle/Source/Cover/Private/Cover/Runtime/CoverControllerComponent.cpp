#include "Cover/Runtime/CoverControllerComponent.h"

#include "Cover/Animation/CoverAnimInterface.h"
#include "Cover/Debug/CoverDebug.h"
#include "Cover/Components/CoverLineComponent.h"
#include "Cover/Runtime/CoverRegistrySubsystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
    constexpr float SnapInterpSpeed = 10.f;
    constexpr float RotInterpSpeed = 12.f;
}

UCoverControllerComponent::UCoverControllerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    SnapMaxDistance = 170.f;
    SnapMaxAngleDeg = 38.f;
    WallMargin = 10.f;
    SlideSpeed = 240.f;
    OverPeekHeadRaise = 22.f;
    SidePeekNudge = 3.f;
    EndPeekWindow = 60.f;

    ShoulderOffset = 22.f;
    SidePeekCamShift = 8.f;
    OverPeekCamLift = 7.f;
    bRightShoulder = true;

    CurrentType = ECoverType::Low;
    State = ESimpleCoverState::Free;
    CurrentLineOwner = nullptr;
    LineT = 0.5f;
    bIsAiming = false;
    CurrentCameraXOffset = 0.f;
    CurrentCameraZOffset = 0.f;

    EnterCoverRotation = FRotator::ZeroRotator;
    SlideDirectionSign = 1.f;
    bFlipCoverSides = false;
    CachedSlideInput = 0.f;
    SnapElapsed = 0.f;
}

void UCoverControllerComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* World = GetWorld())
    {
        RegistryCached = World->GetSubsystem<UCoverRegistrySubsystem>();
    }
}

void UCoverControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearCoverLine();
    Super::EndPlay(EndPlayReason);
}

void UCoverControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const int32 DebugMode = CVarCoverDebug.GetValueOnGameThread();
    if (DebugMode > 0)
    {
        if (UWorld* World = GetWorld())
        {
            UCoverRegistrySubsystem* Registry = RegistryCached.Get();
            if (!Registry)
            {
                Registry = World->GetSubsystem<UCoverRegistrySubsystem>();
                RegistryCached = Registry;
            }

            if (Registry)
            {
                Registry->DrawDebug(World, DebugMode);
            }
        }
    }

    if (State == ESimpleCoverState::Free)
    {
        CurrentCameraXOffset = 0.f;
        CurrentCameraZOffset = 0.f;
        return;
    }

    if (!IsValid(CurrentLineOwner))
    {
        ChangeState(ESimpleCoverState::Free);
        ClearCoverLine();
        return;
    }

    if (CurrentLineOwner)
    {
        CurrentLine = CurrentLineOwner->ToCoverLine();
    }

    SnapElapsed += DeltaTime;

    UpdateInCover(DeltaTime);
    UpdatePeeks();
    UpdateCameraOffsets();

    const FTransform DesiredTransform = ComputeDesiredTransform();
    AActor* OwnerActor = GetOwner();
    if (OwnerActor)
    {
        const FVector CurrentLocation = OwnerActor->GetActorLocation();
        const FRotator CurrentRotation = OwnerActor->GetActorRotation();

        const FVector TargetLocation = DesiredTransform.GetLocation();
        const FRotator TargetRotation = DesiredTransform.Rotator();

        const FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, SnapInterpSpeed);
        const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotInterpSpeed);

        OwnerActor->SetActorLocationAndRotation(NewLocation, NewRotation);
    }

    FCoverDebug::Draw(*this);

    if (State == ESimpleCoverState::SnapIn && SnapElapsed > KINDA_SMALL_NUMBER)
    {
        ChangeState(ESimpleCoverState::InCover);
    }
    else if (State == ESimpleCoverState::Exit)
    {
        ChangeState(ESimpleCoverState::Free);
        ClearCoverLine();
    }
}

bool UCoverControllerComponent::TryEnterCover()
{
    if (State != ESimpleCoverState::Free)
    {
        if (GEngine)
        {
            const FString Message = FString::Printf(TEXT("CoverController: Reject enter. State=%d"), static_cast<int32>(State));
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Message);
        }
        return false;
    }

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("CoverController: Reject enter. No owner actor."));
        }
        return false;
    }

    const FVector ActorLocation = OwnerActor->GetActorLocation();
    const FVector Forward = OwnerActor->GetActorForwardVector();

    FCoverLine Line;
    UCoverLineComponent* OwnerComponent = nullptr;
    if (!QueryBestLine(ActorLocation, Forward, Line, OwnerComponent))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Silver, TEXT("CoverController: Reject enter. No cover line."));
        }
        return false;
    }

    float InitialT = 0.5f;
    FCoverLine::ClosestPointOnSegment(ActorLocation, Line.Start, Line.End, &InitialT);

    SnapToLine(Line, OwnerComponent, InitialT);
    SnapElapsed = 0.f;

    if (GEngine)
    {
        const FString Message = FString::Printf(TEXT("CoverController: Enter success. Type=%d"), static_cast<int32>(CurrentType));
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, Message);
    }

    return true;
}

void UCoverControllerComponent::ExitCover()
{
    if (State == ESimpleCoverState::Free)
    {
        return;
    }

    ChangeState(ESimpleCoverState::Exit);
}

void UCoverControllerComponent::SetAiming(bool bAiming)
{
    bIsAiming = bAiming;

    if (!bIsAiming)
    {
        if (State == ESimpleCoverState::PeekSideLeft || State == ESimpleCoverState::PeekSideRight || State == ESimpleCoverState::PeekOver)
        {
            ChangeState(ESimpleCoverState::InCover);
        }
    }

    UpdateCameraOffsets();
}

void UCoverControllerComponent::AddSlideInput(float AxisX)
{
    CachedSlideInput = AxisX * SlideDirectionSign;
}

bool UCoverControllerComponent::IsMuzzleClear(const FVector& Muzzle, const FVector& AimPoint) const
{
    if (State == ESimpleCoverState::Free)
    {
        return true;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return true;
    }

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(CoverMuzzleTrace), false, GetOwner());
    const bool bHit = World->LineTraceSingleByChannel(Hit, Muzzle, AimPoint, ECC_Visibility, Params);

    if (!bHit)
    {
        return true;
    }

    const float DistanceToWall = FVector::PointPlaneDist(Hit.ImpactPoint, CurrentLine.Start, CurrentLine.Normal);
    const float FacingDot = FVector::DotProduct(Hit.ImpactNormal, CurrentLine.Normal);

    // 같은 벽과 충돌했다면 약간의 여유치를 두고 막힌 것으로 처리.
    if (FacingDot > 0.5f && FMath::Abs(DistanceToWall) <= WallMargin + 5.f)
    {
        return false;
    }

    return true;
}

bool UCoverControllerComponent::QueryBestLine(const FVector& From, const FVector& Fwd, FCoverLine& OutLine, UCoverLineComponent*& OutOwner) const
{
    UCoverRegistrySubsystem* Registry = RegistryCached.Get();
    if (!Registry)
    {
        if (UWorld* World = GetWorld())
        {
            Registry = World->GetSubsystem<UCoverRegistrySubsystem>();
            RegistryCached = Registry;
        }
    }

    if (!Registry)
    {
        return false;
    }

    return Registry->FindBestCoverLine(From, Fwd, SnapMaxDistance, SnapMaxAngleDeg, OutLine, OutOwner);
}

void UCoverControllerComponent::SnapToLine(const FCoverLine& Line, UCoverLineComponent* OwnerComp, float InitialT)
{
    CurrentLine = Line;
    CurrentLineOwner = OwnerComp;
    CurrentType = Line.Type;
    LineT = FMath::Clamp(InitialT, 0.f, 1.f);

    SlideDirectionSign = 1.f;
    bFlipCoverSides = false;

    if (AActor* OwnerActor = GetOwner())
    {
        EnterCoverRotation = OwnerActor->GetActorRotation();
        EnterCoverRotation.Pitch = 0.f;
        EnterCoverRotation.Roll = 0.f;
    }
    else
    {
        EnterCoverRotation = Line.Normal.Rotation();
    }

    const FVector LineVector = Line.End - Line.Start;
    if (!LineVector.IsNearlyZero())
    {
        const FVector Tangent = LineVector.GetSafeNormal();
        FVector EntryRight = EnterCoverRotation.RotateVector(FVector::RightVector);
        if (!EntryRight.IsNearlyZero())
        {
            EntryRight = EntryRight.GetSafeNormal();
            const float Dot = FVector::DotProduct(EntryRight, Tangent);
            if (FMath::Abs(Dot) > KINDA_SMALL_NUMBER)
            {
                SlideDirectionSign = FMath::Sign(Dot);
            }
        }
    }

    const FVector LineNormal = Line.Normal.GetSafeNormal();
    if (!LineNormal.IsNearlyZero())
    {
        FVector EntryForward = EnterCoverRotation.RotateVector(FVector::ForwardVector);
        if (!EntryForward.IsNearlyZero())
        {
            EntryForward = EntryForward.GetSafeNormal();
            bFlipCoverSides = FVector::DotProduct(EntryForward, LineNormal) < 0.f;
        }
    }

    if (bFlipCoverSides)
    {
        SlideDirectionSign *= -1.f;
    }

    ChangeState(ESimpleCoverState::SnapIn);
    SnapElapsed = 0.f;
}

void UCoverControllerComponent::UpdateInCover(float DeltaSeconds)
{
    if (State == ESimpleCoverState::Exit)
    {
        CachedSlideInput = 0.f;
        return;
    }

    if (!CurrentLineOwner)
    {
        return;
    }

    const FVector LineVector = CurrentLine.End - CurrentLine.Start;
    const float LineLength = LineVector.Size();
    if (LineLength < KINDA_SMALL_NUMBER)
    {
        return;
    }

    const float Direction = FMath::Clamp(CachedSlideInput, -1.f, 1.f);
    if (!FMath::IsNearlyZero(Direction))
    {
        const float DeltaT = (Direction * SlideSpeed * DeltaSeconds) / LineLength;
        LineT = FMath::Clamp(LineT + DeltaT, 0.f, 1.f);
    }

    CachedSlideInput = 0.f;
}

void UCoverControllerComponent::UpdatePeeks()
{
    if (State == ESimpleCoverState::SnapIn || State == ESimpleCoverState::Exit)
    {
        return;
    }

    ESimpleCoverState TargetState = ESimpleCoverState::InCover;

    const FVector LineVector = CurrentLine.End - CurrentLine.Start;
    const float LineLength = LineVector.Size();
    const float DistanceLeft = LineLength * LineT;
    const float DistanceRight = LineLength * (1.f - LineT);

    float DistanceToLeftEdge = DistanceLeft;
    float DistanceToRightEdge = DistanceRight;
    bool bLeftOpen = CurrentLine.bLeftOpen;
    bool bRightOpen = CurrentLine.bRightOpen;

    if (bFlipCoverSides)
    {
        const float TempDistance = DistanceToLeftEdge;
        DistanceToLeftEdge = DistanceToRightEdge;
        DistanceToRightEdge = TempDistance;

        const bool TempOpen = bLeftOpen;
        bLeftOpen = bRightOpen;
        bRightOpen = TempOpen;
    }

    if (bIsAiming)
    {
        if (CurrentType == ECoverType::Low)
        {
            TargetState = ESimpleCoverState::PeekOver;
        }
        else if (CurrentType == ECoverType::High)
        {
            const bool bAllowLeft = bLeftOpen && DistanceToLeftEdge <= EndPeekWindow;
            const bool bAllowRight = bRightOpen && DistanceToRightEdge <= EndPeekWindow;

            if (bAllowLeft && (!bAllowRight || DistanceToLeftEdge <= DistanceToRightEdge))
            {
                TargetState = ESimpleCoverState::PeekSideLeft;
            }
            else if (bAllowRight)
            {
                TargetState = ESimpleCoverState::PeekSideRight;
            }
        }
    }

    if (State != TargetState)
    {
        ChangeState(TargetState);
    }
}

FTransform UCoverControllerComponent::ComputeDesiredTransform() const
{
    const FVector LineVector = CurrentLine.End - CurrentLine.Start;
    const FVector Tangent = LineVector.IsNearlyZero() ? FVector::RightVector : LineVector.GetSafeNormal();
    const FVector BasePoint = FMath::Lerp(CurrentLine.Start, CurrentLine.End, LineT);

    FVector DesiredLocation = BasePoint + CurrentLine.Normal * WallMargin;

    if (AActor* OwnerActor = GetOwner())
    {
        FVector ActorLocation = OwnerActor->GetActorLocation();
        DesiredLocation.Z = ActorLocation.Z;
    }

    FVector AdditionalOffset = FVector::ZeroVector;
    if (State == ESimpleCoverState::PeekSideLeft)
    {
        AdditionalOffset += -Tangent * SidePeekNudge;
    }
    else if (State == ESimpleCoverState::PeekSideRight)
    {
        AdditionalOffset += Tangent * SidePeekNudge;
    }

    DesiredLocation += AdditionalOffset;

    FRotator DesiredRotation = EnterCoverRotation;
    DesiredRotation.Normalize();

    if (State == ESimpleCoverState::PeekSideLeft)
    {
        DesiredRotation.Yaw -= 6.f;
    }
    else if (State == ESimpleCoverState::PeekSideRight)
    {
        DesiredRotation.Yaw += 6.f;
    }

    #if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    ensureMsgf(FMath::IsFinite(DesiredLocation.X) && FMath::IsFinite(DesiredLocation.Y) && FMath::IsFinite(DesiredLocation.Z), TEXT("Invalid cover transform"));
    #endif

    return FTransform(DesiredRotation, DesiredLocation);
}

FVector UCoverControllerComponent::GetCurrentTangent() const
{
    const FVector LineVector = CurrentLine.End - CurrentLine.Start;
    if (LineVector.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }

    return LineVector.GetSafeNormal();
}

FVector UCoverControllerComponent::GetCurrentNormal() const
{
    if (CurrentLine.Normal.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }

    return CurrentLine.Normal.GetSafeNormal();
}

bool UCoverControllerComponent::IsCoverSideFlipped() const
{
    return bFlipCoverSides;
}

void UCoverControllerComponent::ChangeState(ESimpleCoverState NewState)
{
    if (State == NewState)
    {
        return;
    }

    State = NewState;

    OnStateChanged.Broadcast(State);

    if (AActor* OwnerActor = GetOwner())
    {
        if (OwnerActor->GetClass()->ImplementsInterface(UCoverAnimInterface::StaticClass()))
        {
            ICoverAnimInterface::Execute_BP_OnCoverStateChanged(OwnerActor, State, CurrentType);
        }
    }
}

void UCoverControllerComponent::ClearCoverLine()
{
    CurrentLineOwner = nullptr;
    CurrentLine = FCoverLine();
    LineT = 0.5f;
    CurrentType = ECoverType::Low;
    SlideDirectionSign = 1.f;
    EnterCoverRotation = FRotator::ZeroRotator;
    bFlipCoverSides = false;
}

void UCoverControllerComponent::UpdateCameraOffsets()
{
    float ShoulderSign = bRightShoulder ? 1.f : -1.f;

    float DesiredX = ShoulderOffset * ShoulderSign;
    float DesiredZ = 0.f;

    if (State == ESimpleCoverState::PeekSideLeft)
    {
        DesiredX -= SidePeekCamShift;
    }
    else if (State == ESimpleCoverState::PeekSideRight)
    {
        DesiredX += SidePeekCamShift;
    }

    if (State == ESimpleCoverState::PeekOver)
    {
        DesiredZ += OverPeekCamLift;
    }

    CurrentCameraXOffset = DesiredX;
    CurrentCameraZOffset = DesiredZ;
}
