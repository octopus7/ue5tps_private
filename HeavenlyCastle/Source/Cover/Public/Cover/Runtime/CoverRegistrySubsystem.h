#pragma once

#include "CoreMinimal.h"
#include "Cover/CoverTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "CoverRegistrySubsystem.generated.h"

class UCoverLineComponent;

/** 월드 내 커버 라인 컴포넌트를 관리하는 간단한 레지스트리. */
UCLASS()
class COVER_API UCoverRegistrySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    void Register(UCoverLineComponent* LineComponent);
    void Unregister(UCoverLineComponent* LineComponent);

    bool FindBestCoverLine(const FVector& From, const FVector& Forward, float MaxDist, float MaxAngleDeg,
        FCoverLine& OutLine, UCoverLineComponent*& OutOwner) const;

private:
    UPROPERTY()
    TArray<TWeakObjectPtr<UCoverLineComponent>> RegisteredLines;
};
