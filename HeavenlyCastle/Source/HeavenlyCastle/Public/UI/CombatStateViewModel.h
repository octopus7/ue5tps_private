// ViewModel for combat state UI

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TPSPlayer.h" // for ECombatState
#include "CombatStateViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, ECombatState, NewState);

UCLASS(BlueprintType)
class HEAVENLYCASTLE_API UCombatStateViewModel : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CombatState = ECombatState::Unarmed;

    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnCombatStateChanged OnCombatStateChanged;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState InState)
    {
        if (CombatState != InState)
        {
            CombatState = InState;
            OnCombatStateChanged.Broadcast(CombatState);
        }
    }
};
