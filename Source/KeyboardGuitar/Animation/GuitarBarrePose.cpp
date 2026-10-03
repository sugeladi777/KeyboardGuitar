#include "GuitarBarrePose.h"

#include "Guitar/GuitarInstrument.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"

namespace
{
    FTransform GetReferenceTransform(const FReferenceSkeleton& Skeleton, FName BoneName)
    {
        FTransform Transform = FTransform::Identity;
        int32 BoneIndex = Skeleton.FindBoneIndex(BoneName);

        // 骨骼保存的是相对父骨骼的变换；沿父级累乘，得到人物网格空间的变换。
        while (BoneIndex != INDEX_NONE)
        {
            Transform = Transform * Skeleton.GetRefBonePose()[BoneIndex];
            BoneIndex = Skeleton.GetParentIndex(BoneIndex);
        }
        return Transform;
    }
}

void FGuitarBarrePose::Initialize(const USkeletalMeshComponent* Mesh)
{
    // 动画资产预览可能还没有指定人物网格。
    if (!Mesh->GetSkeletalMeshAsset())
    {
        return;
    }

    const FReferenceSkeleton& Skeleton = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();
    const FTransform Bones[] = {
        GetReferenceTransform(Skeleton, TEXT("Bip01-L-Finger1")),
        GetReferenceTransform(Skeleton, TEXT("Bip01-L-Finger11")),
        GetReferenceTransform(Skeleton, TEXT("Bip01-L-Finger12"))
    };
    const FTransform Hand = GetReferenceTransform(Skeleton, TEXT("Bip01-L-Hand"));
    RootInHand = Bones[0].GetRelativeTransform(Hand).GetLocation();
    MiddleInParent = Skeleton.GetRefBonePose()[Skeleton.FindBoneIndex(TEXT("Bip01-L-Finger11"))].GetLocation();
    EndInParent = Skeleton.GetRefBonePose()[Skeleton.FindBoneIndex(TEXT("Bip01-L-Finger12"))].GetLocation();

    const FVector FirstDirection = (Bones[1].GetLocation() - Bones[0].GetLocation()).GetSafeNormal();
    const FVector SecondDirection = (Bones[2].GetLocation() - Bones[1].GetLocation()).GetSafeNormal();
    // 参考姿势中指节弯向手掌；去掉沿第一节的分量，得到指腹朝向。
    const FVector PalmDirection = (SecondDirection - FirstDirection *
        FVector::DotProduct(FirstDirection, SecondDirection)).GetSafeNormal();

    for (int32 Joint = 0; Joint < 3; ++Joint)
    {
        const FVector Direction = Joint < 2 ?
            (Bones[Joint + 1].GetLocation() - Bones[Joint].GetLocation()).GetSafeNormal() :
            Bones[Joint].TransformVectorNoScale(FVector::ForwardVector);
        const FQuat Frame = FRotationMatrix::MakeFromXZ(Direction, PalmDirection).ToQuat();
        // 模型骨轴不一定等于指向和指腹方向，保留二者之间的旋转差。
        RotationOffsets[Joint] = Frame.Inverse() * Bones[Joint].GetRotation();
        if (Joint < 2)
        {
            JointLengths[Joint] = FVector::Distance(Bones[Joint].GetLocation(), Bones[Joint + 1].GetLocation());
        }
    }
}

FVector FGuitarBarrePose::Calculate(const AGuitarInstrument* Guitar, const TArray<FVector>& Contacts,
    const FTransform& MeshTransform, const FTransform& HandTarget)
{
    const FVector Direction = (Contacts.Last() - Contacts[0]).GetSafeNormal();
    const FVector NeckDirection = (Guitar->GetFretPosition(0, 0) - Guitar->GetFretPosition(0, 12)).GetSafeNormal();
    const FVector FaceNormal = FVector::CrossProduct(NeckDirection, -Direction).GetSafeNormal();

    // 食指从细弦伸向粗弦；指节中心高出弦面 0.45 cm，让指腹贴弦。
    const FVector Tip = Contacts.Last() + FaceNormal * 0.45f;
    const FVector Root = Tip - Direction * (JointLengths[0] + JointLengths[1] + JointLengths[2]);
    const FVector LocalDirection = MeshTransform.InverseTransformVectorNoScale(Direction).GetSafeNormal();
    const FVector LocalPalm = MeshTransform.InverseTransformVectorNoScale(-FaceNormal).GetSafeNormal();
    const FQuat Frame = FRotationMatrix::MakeFromXZ(LocalDirection, LocalPalm).ToQuat();

    float Distance = 0.0f;
    for (int32 Joint = 0; Joint < 3; ++Joint)
    {
        MeshSpaceJoints[Joint] = FTransform(Frame * RotationOffsets[Joint],
            MeshTransform.InverseTransformPosition(Root + Direction * Distance));
        Distance += JointLengths[Joint];
    }

    // 移动手腕来对齐指根，避免把食指从手掌上拉开。
    const FVector HandPosition = MeshSpaceJoints[0].GetLocation() -
        HandTarget.GetRotation().RotateVector(RootInHand);
    return HandPosition - HandTarget.GetLocation();
}

void FGuitarBarrePose::GetLocalTransforms(const FTransform& HandTarget,
    FTransform& Root, FTransform& Middle, FTransform& End) const
{
    Root = MeshSpaceJoints[0].GetRelativeTransform(HandTarget);
    Middle = MeshSpaceJoints[1].GetRelativeTransform(MeshSpaceJoints[0]);
    End = MeshSpaceJoints[2].GetRelativeTransform(MeshSpaceJoints[1]);

    // Control Rig 混合局部旋转时，平移仍使用参考骨长，避免过渡中拉伸手指。
    Root.SetTranslation(RootInHand);
    Middle.SetTranslation(MiddleInParent);
    End.SetTranslation(EndInParent);
}
