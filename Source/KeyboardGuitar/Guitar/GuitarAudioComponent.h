#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuitarAudioComponent.generated.h"

class UAudioComponent;
class UGuitarData;

/** 管理六根弦的音频采样；各弦独立发声，同一根弦重复拨动时重新播放。 */
UCLASS(ClassGroup = (Guitar))
class KEYBOARDGUITAR_API UGuitarAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGuitarAudioComponent();

    /** 各弦共用的音量倍率，范围为 0～1。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guitar|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StringVolume = 0.7f;

    /** 游戏开始后调用；弦索引 0～5，MidiNote 是采样表中的音高编号。 */
    void PlayNote(int32 StringIndex, int32 MidiNote, const UGuitarData* Data);

    /** 停止这根弦的声音，不影响其他弦。 */
    void StopString(int32 StringIndex);

    /** 停止六路声音。 */
    void StopAll();

protected:
    virtual void BeginPlay() override;

private:
    /** BeginPlay 中创建，顺序对应第一弦到第六弦，不保存到资源中。 */
    UPROPERTY(Transient)
    TArray<TObjectPtr<UAudioComponent>> Voices;
};
