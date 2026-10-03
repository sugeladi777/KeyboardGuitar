#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GuitarData.generated.h"

class USoundWave;

/** 一个和弦的名称与六根弦的指法。 */
USTRUCT(BlueprintType)
struct FChordData
{
    GENERATED_BODY()

    /** 和弦名称，例如 C、Am、G7。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName ChordName;

    /** 固定六项，索引 0～5 对应第一弦到第六弦；-1 静音，0 空弦，正数表示品位。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<int32> Frets = { 0, 0, 0, 0, 0, 0 };

    /**
     * 与 Frets 一一对应：0 不按弦，1 食指，2 中指，3 无名指，4 小指。
     * 同一手指可出现在同一品位的多根弦上，表示横按；空弦与静音弦填 0。
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "4"))
    TArray<int32> Fingers = { 0, 0, 0, 0, 0, 0 };
};

/** 在数据资产中配置演奏数据，供和弦查询和音频播放共用。 */
UCLASS(BlueprintType)
class KEYBOARDGUITAR_API UGuitarData : public UDataAsset
{
    GENERATED_BODY()

public:
    /** 第一弦到第六弦的空弦 MIDI 音高；每升一个品位，音高编号增加 1。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guitar")
    TArray<int32> OpenStringNotes = { 64, 59, 55, 50, 45, 40 };

    /** 和弦列表；输入动作中的和弦索引对应此数组的索引。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guitar")
    TArray<FChordData> Chords;

    /** 键为 MIDI 音高编号，值为对应的音频采样。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TMap<int32, TObjectPtr<USoundWave>> NoteSamples;
};
