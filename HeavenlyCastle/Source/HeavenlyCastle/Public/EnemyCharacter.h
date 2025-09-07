#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UTextRenderComponent;
class UHealthComponent;

// 간이 적 캐릭터: 뒤집힌 콘 모양의 시각화만 제공
UCLASS(meta=(DisplayName="EnemyCharacter_임시"))
class HEAVENLYCASTLE_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UTextRenderComponent* HPText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UTextRenderComponent* LifeText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UHealthComponent* HealthComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
    int32 HitsToDie = 3;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Health")
    int32 CurrentHits = 0;

    // 자동 사망 시간(초). 0이면 자동 사망하지 않음. 기본 10초.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lifetime", meta=(ClampMin="0.0"))
    float AutoDeathTime = 0.0f;

    void HandleDeath();
    void UpdateHPText();
    void UpdateLifetimeText();

    UFUNCTION()
    void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    UPROPERTY(Transient)
    class AEnemyAIController* CachedAIController = nullptr;

    // (제거됨) 플레이어까지 핑크 디버그 라인 표시 옵션
};
