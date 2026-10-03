#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuitarChordComponent.generated.h"

class UGuitarData;

/** 管理和弦按住状态和品位查询；最后按下的和弦优先。 */
UCLASS(ClassGroup = (Guitar))
class KEYBOARDGUITAR_API UGuitarChordComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGuitarChordComponent();

    /** ChordIndex 对应 GuitarData.Chords 的数组索引。 */
    void HoldChord(int32 ChordIndex);

    /** 释放指定和弦；仍有其他按住的和弦时，恢复最后按下的一个。 */
    void ReleaseChord(int32 ChordIndex);

    /** 是否有至少一个和弦处于按住状态。 */
    bool IsHoldingChord() const;

    /** 弦索引 0～5；返回 -1（静音）、0（空弦）或正数品位。 */
    int32 GetFretForString(int32 StringIndex, const UGuitarData* Data) const;

    /** 返回当前和弦在这根弦上使用的手指；0 表示不按弦。 */
    int32 GetFingerForString(int32 StringIndex, const UGuitarData* Data) const;

private:
    /** 按下顺序从前到后排列，末尾是当前生效的和弦。 */
    TArray<int32> HeldChordIndices;
};
