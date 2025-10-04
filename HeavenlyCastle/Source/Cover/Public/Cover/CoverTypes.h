#pragma once

#include "CoreMinimal.h"
#include "CoverTypes.generated.h"

UENUM(BlueprintType)
enum class ECoverType : uint8
{
    Low UMETA(DisplayName = "Low"),
    High UMETA(DisplayName = "High")
};

UENUM(BlueprintType)
enum class ESimpleCoverState : uint8
{
    Free UMETA(DisplayName = "Free"),
    SnapIn UMETA(DisplayName = "SnapIn"),
    InCover UMETA(DisplayName = "InCover"),
    PeekSideLeft UMETA(DisplayName = "PeekSideLeft"),
    PeekSideRight UMETA(DisplayName = "PeekSideRight"),
    PeekOver UMETA(DisplayName = "PeekOver"),
    Exit UMETA(DisplayName = "Exit")
};

USTRUCT(BlueprintType)
struct FCoverLine
{
    GENERATED_BODY()

    FCoverLine()
        : Start(FVector::ZeroVector)
        , End(FVector::ZeroVector)
        , Normal(FVector::XAxisVector)
        , Type(ECoverType::Low)
        , bLeftOpen(true)
        , bRightOpen(true)
        , Height(100.f)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
    FVector Start;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
    FVector End;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
    FVector Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
    ECoverType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
    bool bLeftOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
    bool bRightOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
    float Height;

    static FVector ClosestPointOnSegment(const FVector& P, const FVector& A, const FVector& B, float* OutT = nullptr)
    {
        const FVector AB = B - A;
        const float Denominator = FVector::DotProduct(AB, AB);
        float T = 0.f;

        if (Denominator > KINDA_SMALL_NUMBER)
        {
            T = FVector::DotProduct(P - A, AB) / Denominator;
            T = FMath::Clamp(T, 0.f, 1.f);
        }

        if (OutT)
        {
            *OutT = T;
        }

        return A + AB * T;
    }

    static bool IsNearEnd(float T, float Threshold = 0.08f)
    {
        return (T <= Threshold) || (T >= (1.f - Threshold));
    }
};
