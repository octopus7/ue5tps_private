// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimpleHUDWidget.generated.h"

class UCanvasPanel;
class UVerticalBox;
class UTextBlock;
class UProgressBar;

UCLASS()
class HEAVENLYCASTLE_API USimpleHUDWidget : public UUserWidget
{
	GENERATED_BODY()	

public:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

protected:
    void BuildIfNeeded();

private:
    UPROPERTY()
    UCanvasPanel* RootCanvas = nullptr;

    /** Container created at runtime to hold text labels. */
    UPROPERTY(Transient)
    UVerticalBox* RootLayout;

    UPROPERTY()
    UVerticalBox* TopBox = nullptr;

    UPROPERTY()
    UTextBlock* TimeText = nullptr;

    UPROPERTY()
    UTextBlock* HPText = nullptr;

    UPROPERTY()
    UProgressBar* HPBar = nullptr;
	
};
