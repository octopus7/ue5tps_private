
#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "EnemyAIState.generated.h"

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
    EAS_Idle      UMETA(DisplayName = "Idle"),
    EAS_Patrol    UMETA(DisplayName = "Patrol"),
    EAS_Chase     UMETA(DisplayName = "Chase"),
    EAS_Attack    UMETA(DisplayName = "Attack"),
    EAS_Dead      UMETA(DisplayName = "Dead")
};
