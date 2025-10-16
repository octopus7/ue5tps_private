#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BoxPlayerStatusWidget.generated.h"

class UTextBlock;

/**
 * BoxPlayer 위에 표시할 간단한 텍스트 위젯
 */
UCLASS()
class HEAVENLYCASTLE_API UBoxPlayerStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UBoxPlayerStatusWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY()
    TObjectPtr<UTextBlock> StatusText;

    void InitializeText(UTextBlock* InText);
};

