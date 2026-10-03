#pragma once

#include "CoreMinimal.h"

class AGuitarInstrument;
class USkeletalMeshComponent;

/**
 * 横按姿势计算：读取食指骨长，让伸直的食指覆盖同品位的多根弦。
 * 这是普通 C++ 计算对象；动画实例负责调用它，并把结果传给 Control Rig。
 */
struct FGuitarBarrePose
{
    /** 从人物参考姿势读取骨长和指腹方向，在动画初始化时调用。 */
    void Initialize(const USkeletalMeshComponent* Mesh);

    /**
     * 计算网格空间食指姿势，返回手腕需要移动的网格空间偏移。
     * Contacts 至少两项，使用世界坐标，按细弦到粗弦排列。
     */
    FVector Calculate(const AGuitarInstrument* Guitar, const TArray<FVector>& Contacts,
        const FTransform& MeshTransform, const FTransform& HandTarget);

    /** Root 相对手掌，Middle 相对 Root，End 相对 Middle；保持原始骨长。 */
    void GetLocalTransforms(const FTransform& HandTarget,
        FTransform& Root, FTransform& Middle, FTransform& End) const;

private:
    /** 前两段长度来自模型；最后 1.2 cm 对应 Control Rig 中的指尖接触骨骼。 */
    float JointLengths[3] = { 0.0f, 0.0f, 1.2f };
    FQuat RotationOffsets[3];
    FVector RootInHand = FVector::ZeroVector;
    FVector MiddleInParent = FVector::ZeroVector;
    FVector EndInParent = FVector::ZeroVector;

    /** Calculate 写入网格空间姿势；GetLocalTransforms 只转换输出，不修改缓存。 */
    FTransform MeshSpaceJoints[3];
};
