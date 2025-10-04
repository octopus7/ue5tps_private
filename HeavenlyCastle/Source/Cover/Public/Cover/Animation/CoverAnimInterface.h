#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Cover/CoverTypes.h"
#include "CoverAnimInterface.generated.h"

UINTERFACE(BlueprintType, Blueprintable)
class COVER_API UCoverAnimInterface : public UInterface
{
    GENERATED_BODY()
};

class COVER_API ICoverAnimInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "Cover")
    void BP_OnCoverStateChanged(ESimpleCoverState NewState, ECoverType Type);
};
