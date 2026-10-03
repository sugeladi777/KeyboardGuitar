#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuitarPickingComponent.generated.h"

/** 管理一次拨弦的准备、触弦和恢复，并为右手提供指尖目标。 */
UCLASS(ClassGroup = (Guitar))
class KEYBOARDGUITAR_API UGuitarPickingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGuitarPickingComponent();

    /** 按键到触弦的秒数；声音与琴弦振动在触弦时一起触发。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guitar|Picking", meta = (ClampMin = "0.01", ClampMax = "0.1"))
    float ContactTime = 0.04f;

    /** 触弦后恢复到准备姿势的秒数。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guitar|Picking", meta = (ClampMin = "0.06"))
    float RecoveryTime = 0.20f;

    /** 开始一次拨弦；弦索引 0～5，Fret 为当次拨弦记录的非负品位。 */
    void StartPick(int32 StringIndex, int32 Fret);

    /** 取消这根弦尚未完成的动作与待发声音。 */
    void CancelString(int32 StringIndex);

    /** 取消全部拨弦，关闭 Tick；声音和振动由 GuitarInstrument 一起停止。 */
    void StopAll();

    /** 返回世界空间指尖位置；手指索引 0 拇指、1 食指、2 中指、3 无名指。 */
    FVector GetFingerPosition(int32 Finger) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    struct FPickState
    {
        float ElapsedTime = 0.0f; // 本次拨弦已过去的秒数。
        int32 Fret = 0; // 按键时的品位。
        bool bActive = false; // 动作是否仍在进行。
        bool bSoundPending = false; // 触弦前为 true，发声或取消后为 false。
        FVector StartPosition = FVector::ZeroVector;
    };

    /** 每根弦独立记录触弦时刻，快速拨不同低音弦时仍保留各自的声音。 */
    FPickState States[6];

    /** 每根手指当前负责的弦；拇指会在三个低音弦之间切换。 */
    int32 FingerStrings[4] = { 5, 2, 1, 0 };

    FVector GetFaceNormal() const;
};
