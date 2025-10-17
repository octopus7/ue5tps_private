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
    void SetAmmoCounts(int32 MagazineAmmo, int32 ReserveAmmo);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY()
    TObjectPtr<UTextBlock> CoverStatusText;

    UPROPERTY()
    TObjectPtr<UTextBlock> MagazineText;

    UPROPERTY()
    TObjectPtr<UTextBlock> ReserveText;

    bool bIsCoverAvailable;
    int32 CachedMagazineAmmo;
    int32 CachedReserveAmmo;

    void InitializeCoverText(UTextBlock* InText);
    void InitializeAmmoText(UTextBlock* InText, bool bIsMagazine);
    void RefreshText();
    void RefreshAmmoTexts();
};
