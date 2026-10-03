#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GuitarBarrePose.h"
#include "GuitarPerformerAnimInstance.generated.h"

class USceneComponent;
class AGuitarInstrument;
class UGuitarPickingComponent;

/** 动画数据桥接：读取吉他与人物定位点，向动画蓝图和 Control Rig 提供姿势目标。 */
UCLASS()
class KEYBOARDGUITAR_API UGuitarPerformerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    /** 左手腕的网格空间变换，包含和弦跟随偏移。 */
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Pose")
    FTransform LeftHandTarget = FTransform::Identity;

    /** 右手腕的网格空间变换，来自人物蓝图中的 RightHandTarget。 */
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Pose")
    FTransform RightHandTarget = FTransform::Identity;

    /** 四根按弦手指的指腹目标，使用人物网格空间。 */
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Fingers")
    FTransform IndexTarget;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Fingers")
    FTransform MiddleTarget;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Fingers")
    FTransform RingTarget;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Fingers")
    FTransform LittleTarget;

    /** 0 保持放松姿势，1 完全按弦；过渡值用于平滑切换。 */
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Fingers")
    float IndexWeight = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Fingers")
    float MiddleWeight = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Fingers")
    float RingWeight = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Fingers")
    float LittleWeight = 0.0f;

    /** 横按时食指三个指节的局部变换，覆盖普通指尖 IK。 */
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Barre")
    FTransform BarreRoot;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Barre")
    FTransform BarreMiddle;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Barre")
    FTransform BarreEnd;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Barre")
    float BarreWeight = 0.0f;

    /** 右手指尖的网格空间目标；顺序为拇指、食指、中指、无名指。 */
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Picking")
    FTransform PickThumbTarget;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Picking")
    FTransform PickIndexTarget;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Picking")
    FTransform PickMiddleTarget;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Picking")
    FTransform PickRingTarget;
    UPROPERTY(BlueprintReadOnly, Category = "Guitar|Picking")
    float PickingWeight = 0.0f;

private:
    /** 绑定人物组件，获取吉他；资产预览和子 Actor 创建较晚时允许暂时缺失。 */
    void FindHandTargets();
    void FindGuitar();

    /** 每帧先读手腕定位，再分别计算左手按弦和右手拨弦。 */
    void UpdateHandTargets(const FTransform& MeshTransform);
    void UpdateLeftHand(const FTransform& MeshTransform, float DeltaSeconds);
    void UpdateRightHand(const FTransform& MeshTransform, float DeltaSeconds);
    void UpdateFingerTarget(const TArray<FVector>& Contacts, FTransform& Target,
        float& Weight, const FTransform& MeshTransform, float DeltaSeconds);

    FGuitarBarrePose BarrePose;

    UPROPERTY(Transient)
    TObjectPtr<AGuitarInstrument> Guitar;
    UPROPERTY(Transient)
    TObjectPtr<UGuitarPickingComponent> Picking;

    /** 手腕跟随按弦区域移动，让目标保持在手指可达范围。 */
    FVector HandOffset = FVector::ZeroVector;
    UPROPERTY(Transient)
    TObjectPtr<USceneComponent> HandTarget;

    UPROPERTY(Transient)
    TObjectPtr<USceneComponent> RightHandTargetComponent;
};
