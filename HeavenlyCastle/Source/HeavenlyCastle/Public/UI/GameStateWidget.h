#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameStateWidget.generated.h"

class UTextBlock;

/**
 * 커버 가능 여부를 표시하는 간단한 위젯
 */
UCLASS()
class HEAVENLYCASTLE_API UGameStateWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UGameStateWidget(const FObjectInitializer& ObjectInitializer);

    void SetCoverAvailable(bool bAvailable);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY()
    TObjectPtr<UTextBlock> CoverStatusText;

    bool bIsCoverAvailable;

    void InitializeText(UTextBlock* InText);
    void RefreshText();
};
