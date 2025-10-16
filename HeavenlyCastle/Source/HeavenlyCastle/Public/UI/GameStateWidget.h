#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameStateWidget.generated.h"

class UTextBlock;

/**
 * Displays simple text for game state information
 */
UCLASS()
class HEAVENLYCASTLE_API UGameStateWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UGameStateWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY()
    TObjectPtr<UTextBlock> StateText;

    void InitializeText(UTextBlock* InText);
};

