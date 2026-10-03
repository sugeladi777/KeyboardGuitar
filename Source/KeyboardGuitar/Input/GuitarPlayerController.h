#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GuitarPlayerController.generated.h"

class AGuitarInstrument;
class UInputAction;
class UInputMappingContext;

/** 接收 Enhanced Input 输入，把拨弦和和弦操作转交给场景中的吉他。 */
UCLASS()
class KEYBOARDGUITAR_API AGuitarPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AGuitarPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    /** 在继承蓝图中指定 IMC，定义按键与输入动作的对应关系。 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guitar|Input")
    TObjectPtr<UInputMappingContext> GuitarMappingContext;

    /** 固定六项，索引 0～5 对应第一弦到第六弦的输入动作。 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guitar|Input")
    TArray<TObjectPtr<UInputAction>> StringActions;

    /** 各项索引对应 GuitarData.Chords；当前构造函数默认预留四项。 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guitar|Input")
    TArray<TObjectPtr<UInputAction>> ChordActions;

private:
    /** 当前关卡中的吉他，开始游戏时获取一次。 */
    UPROPERTY()
    TObjectPtr<AGuitarInstrument> Guitar;

    /** 输入回调只转交操作，演奏逻辑由 GuitarInstrument 协调。 */
    void PluckString(int32 StringIndex);
    void HoldChord(int32 ChordIndex);
    void ReleaseChord(int32 ChordIndex);
};
