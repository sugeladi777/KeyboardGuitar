#include "GuitarPlayerController.h"

#include "Guitar/GuitarInstrument.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

AGuitarPlayerController::AGuitarPlayerController()
{
    StringActions.SetNum(6);
    ChordActions.SetNum(4);
}

void AGuitarPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 当前采用单吉他场景，开始游戏时查找一次演奏对象。
    Guitar = Cast<AGuitarInstrument>(UGameplayStatics::GetActorOfClass(this, AGuitarInstrument::StaticClass()));

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    // 为本地玩家启用吉他的输入映射，优先级为 0。
    InputSubsystem->AddMappingContext(GuitarMappingContext, 0);
}

void AGuitarPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);

    // 按下时拨弦一次；绑定时把数组索引作为弦号传给回调。
    for (int32 StringIndex = 0; StringIndex < StringActions.Num(); ++StringIndex)
    {
        UInputAction* Action = StringActions[StringIndex];
        EnhancedInput->BindAction(Action, ETriggerEvent::Started,
            this, &AGuitarPlayerController::PluckString, StringIndex);
    }

    // 和弦只在按住期间生效；正常结束和取消输入时都需要释放。
    for (int32 ChordIndex = 0; ChordIndex < ChordActions.Num(); ++ChordIndex)
    {
        UInputAction* Action = ChordActions[ChordIndex];

        EnhancedInput->BindAction(Action, ETriggerEvent::Started,
            this, &AGuitarPlayerController::HoldChord, ChordIndex);
        EnhancedInput->BindAction(Action, ETriggerEvent::Completed,
            this, &AGuitarPlayerController::ReleaseChord, ChordIndex);
        EnhancedInput->BindAction(Action, ETriggerEvent::Canceled,
            this, &AGuitarPlayerController::ReleaseChord, ChordIndex);
    }
}

void AGuitarPlayerController::PluckString(int32 StringIndex)
{
    Guitar->PlayString(StringIndex);
}

void AGuitarPlayerController::HoldChord(int32 ChordIndex)
{
    Guitar->HoldChord(ChordIndex);
}

void AGuitarPlayerController::ReleaseChord(int32 ChordIndex)
{
    Guitar->ReleaseChord(ChordIndex);
}
