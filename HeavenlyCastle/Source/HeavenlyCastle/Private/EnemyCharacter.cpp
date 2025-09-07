#include "EnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "EnemyAIController.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HealthComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Ensure enemy capsule can overlap with projectile (WorldDynamic)
    GetCapsuleComponent()->SetGenerateOverlapEvents(true);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

    // HP text above head
    HPText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HPText"));
    HPText->SetupAttachment(GetCapsuleComponent());
    HPText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    HPText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    HPText->SetWorldSize(108.f);
    HPText->SetTextRenderColor(FColor::White);
    HPText->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
    UpdateHPText();

    // Lifetime text above HP (50% size)
    LifeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LifeText"));
    LifeText->SetupAttachment(GetCapsuleComponent());
    LifeText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    LifeText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    LifeText->SetWorldSize(54.f);
    LifeText->SetTextRenderColor(FColor::Cyan);
    LifeText->SetRelativeLocation(FVector(0.f, 0.f, 190.f));
    LifeText->SetHiddenInGame(true); // will unhide if AutoDeathTime > 0 in BeginPlay

    // AI setup
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AEnemyAIController::StaticClass();

    // Movement setup
    bUseControllerRotationYaw = false;
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
        MoveComp->bConstrainToPlane = false;
        //MoveComp->SetPlaneConstraintNormal(FVector::UpVector);
    }

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 스폰 후 자동 사망 시간 적용 (0이면 비활성)
    if (AutoDeathTime > 0.f)
    {
        SetLifeSpan(AutoDeathTime);
        if (LifeText)
        {
            LifeText->SetHiddenInGame(false);
            UpdateLifetimeText();
        }
    }
    else
    {
        SetLifeSpan(0.f);
        if (LifeText)
        {
            LifeText->SetHiddenInGame(true);
        }
    }
    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddDynamic(this, &AEnemyCharacter::OnHealthChanged);
    }
    UpdateHPText();
}

void AEnemyCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    CachedAIController = Cast<AEnemyAIController>(NewController);
    UpdateLifetimeText();
}

void AEnemyCharacter::UnPossessed()
{
    Super::UnPossessed();
    CachedAIController = nullptr;
    UpdateLifetimeText();
}

float AEnemyCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Let base class handle built-in events. Real health change happens in UHealthComponent via OnTakeAnyDamage
    const float SuperDealt = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    // Optionally display hit feedback
    if (GEngine)
    {
        FString Msg = FString::Printf(TEXT("Enemy took damage: %.1f"), DamageAmount);
        GEngine->AddOnScreenDebugMessage(reinterpret_cast<uint64>(this), 1.0f, FColor::Red, Msg);
    }
    return SuperDealt + DamageAmount;
}

void AEnemyCharacter::HandleDeath()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Enemy died (ragdoll)"));
    }

    // 기존 랙돌 처리와 유사하게 적용
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
    }

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (USkeletalMeshComponent* Skel = GetMesh())
    {
        // 스켈레탈 메시 랙돌 처리
        Skel->SetSimulatePhysics(true);
        Skel->SetCollisionProfileName(TEXT("Ragdoll"));
        // 랙돌이 다른 Pawn(플레이어/적)을 물리적으로 밀지 않도록 무시
        Skel->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        Skel->SetGenerateOverlapEvents(false);
    }

    // 컨트롤러 무력화 및 분리
    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->StopMovement();
        AI->UnPossess();
    }

    // 잠시 후 제거
    SetLifeSpan(5.0f);
}

void AEnemyCharacter::UpdateHPText()
{
    if (!HPText)
    {
        return;
    }

    // State text (English). If AI is null, show "null"
    FString StateStr;
    AEnemyAIController* AI = CachedAIController ? CachedAIController : Cast<AEnemyAIController>(GetController());
    if (AI)
    {
        switch (AI->GetState())
        {
        case EEnemyState::Idle:   StateStr = TEXT("Idle");   break;
        case EEnemyState::Patrol: StateStr = TEXT("Patrol"); break;
        case EEnemyState::Chase:  StateStr = TEXT("Chase");  break;
        default:                  StateStr = TEXT("Idle");   break;
        }
    }
    else
    {
        StateStr = TEXT("null");
    }

    // Append extra info when Idle/Chase
    FString Extra;
    if (AI)
    {
        if (AI->GetState() == EEnemyState::Idle)
        {
            const float LeftIdle = AI->GetIdleTimeRemaining();
            if (LeftIdle > 0.f)
            {
                Extra = FString::Printf(TEXT(" (%.1fs)"), LeftIdle);
            }
        }
        else if (AI->GetState() == EEnemyState::Chase)
        {
            const float Rem = AI->GetAttackTimeRemaining();
            Extra = FString::Printf(TEXT(" (%.1fs)"), Rem);
        }
    }
    float HP = 0.f, MaxHP = 0.f;
    if (HealthComponent)
    {
        HP = HealthComponent->GetHealth();
        MaxHP = HealthComponent->GetDefaultHealth();
    }
    else
    {
        // Fallback to hits model if no HealthComponent exists
        const int32 Remaining = FMath::Max(0, HitsToDie - CurrentHits);
        HP = (float)Remaining;
        MaxHP = (float)HitsToDie;
    }

    const FString Txt = FString::Printf(TEXT("HP: %.0f/%.0f | %s%s"), HP, MaxHP, *StateStr, *Extra);
    HPText->SetText(FText::FromString(Txt));

    const float Ratio = (MaxHP > 0.f) ? (HP / MaxHP) : 0.f;
    if (Ratio <= 0.33f)
    {
        HPText->SetTextRenderColor(FColor::Red);
    }
    else if (Ratio <= 0.66f)
    {
        HPText->SetTextRenderColor(FColor::Yellow);
    }
    else
    {
        HPText->SetTextRenderColor(FColor::White);
    }
}

void AEnemyCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    UpdateHPText();
    if (OwningHealthComp && OwningHealthComp->IsDead())
    {
        HandleDeath();
    }
}

void AEnemyCharacter::UpdateLifetimeText()
{
    if (!LifeText)
    {
        return;
    }
    const float Left = GetLifeSpan();
    if (Left <= 0.f)
    {
        LifeText->SetHiddenInGame(true);
        return;
    }
    LifeText->SetHiddenInGame(false);
    const int32 Secs = FMath::Max(0, FMath::CeilToInt(Left));
    LifeText->SetText(FText::AsNumber(Secs));
}

void AEnemyCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (AutoDeathTime > 0.f)
    {
        UpdateLifetimeText();
    }
    // 상태는 HP 텍스트에 표시되므로 매 틱 갱신
    UpdateHPText();

    // 머리 위 텍스트를 카메라 방향으로 정렬
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (PC->PlayerCameraManager)
        {
            const FRotator CamRot = PC->PlayerCameraManager->GetCameraRotation();
            // 텍스트가 카메라를 바라보도록 Yaw만 맞춤(+180도로 반전)
            const FRotator FaceRot(0.f, CamRot.Yaw + 180.f, 0.f);
            if (HPText)
            {
                HPText->SetWorldRotation(FaceRot);
            }
            if (LifeText && !LifeText->bHiddenInGame)
            {
                LifeText->SetWorldRotation(FaceRot);
            }
        }
    }

    // (제거됨) 플레이어까지 핑크 디버그 라인 표시 코드
}
