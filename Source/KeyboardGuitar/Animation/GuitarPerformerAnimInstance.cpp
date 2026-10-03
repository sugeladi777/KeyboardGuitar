#include "GuitarPerformerAnimInstance.h"

#include "Guitar/GuitarInstrument.h"
#include "Guitar/GuitarPickingComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UGuitarPerformerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    Guitar = nullptr;
    Picking = nullptr;
    HandTarget = nullptr;
    RightHandTargetComponent = nullptr;
    IndexWeight = MiddleWeight = RingWeight = LittleWeight = 0.0f;
    BarreWeight = PickingWeight = 0.0f;
    HandOffset = FVector::ZeroVector;

    BarrePose.Initialize(GetSkelMeshComponent());
    FindHandTargets();
}

void UGuitarPerformerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    FindGuitar();
    const FTransform MeshTransform = GetSkelMeshComponent()->GetComponentTransform();
    UpdateHandTargets(MeshTransform);
    UpdateLeftHand(MeshTransform, DeltaSeconds);
    UpdateRightHand(MeshTransform, DeltaSeconds);
}

void UGuitarPerformerAnimInstance::FindHandTargets()
{
    // 动画资产预览没有完整演奏者；只有实际人物蓝图上才有这些定位组件。
    AActor* Performer = GetOwningActor();
    if (!Performer)
    {
        return;
    }

    TArray<USceneComponent*> Components;
    Performer->GetComponents(Components);
    for (USceneComponent* Component : Components)
    {
        if (Component->GetFName() == TEXT("LeftHandTarget"))
        {
            HandTarget = Component;
        }
        else if (Component->GetFName() == TEXT("RightHandTarget"))
        {
            RightHandTargetComponent = Component;
        }
    }
}

void UGuitarPerformerAnimInstance::FindGuitar()
{
    if (Guitar || !GetOwningActor())
    {
        return;
    }

    // 子 Actor 可能晚于动画实例创建，因此逐帧查找，找到后缓存。
    TArray<UChildActorComponent*> Children;
    GetOwningActor()->GetComponents(Children);
    for (UChildActorComponent* Child : Children)
    {
        Guitar = Cast<AGuitarInstrument>(Child->GetChildActor());
        if (Guitar)
        {
            Picking = Guitar->FindComponentByClass<UGuitarPickingComponent>();
            // 先更新拨弦时间，再计算人物姿势，让触弦动作和发声处于同一帧。
            GetSkelMeshComponent()->AddTickPrerequisiteComponent(Picking);
            break;
        }
    }
}

void UGuitarPerformerAnimInstance::UpdateHandTargets(const FTransform& MeshTransform)
{
    // Control Rig 的 Global Space 是人物网格空间，不能直接传入世界坐标。
    if (HandTarget)
    {
        LeftHandTarget = HandTarget->GetComponentTransform().GetRelativeTransform(MeshTransform);
    }
    if (RightHandTargetComponent)
    {
        RightHandTarget = RightHandTargetComponent->GetComponentTransform().GetRelativeTransform(MeshTransform);
    }
}

void UGuitarPerformerAnimInstance::UpdateFingerTarget(const TArray<FVector>& Contacts,
    FTransform& Target, float& Weight, const FTransform& MeshTransform, float DeltaSeconds)
{
    const bool bPressing = !Contacts.IsEmpty();
    if (bPressing)
    {
        const FVector Position = MeshTransform.InverseTransformPosition(Contacts[0]);
        // 第一次按弦直接设置目标，避免从原点飞入；后续切换平滑移动。
        Target.SetLocation(Weight < 0.001f ? Position :
            FMath::VInterpTo(Target.GetLocation(), Position, DeltaSeconds, 15.0f));
    }
    Weight = FMath::FInterpTo(Weight, bPressing ? 1.0f : 0.0f, DeltaSeconds, 15.0f);
}

void UGuitarPerformerAnimInstance::UpdateLeftHand(const FTransform& MeshTransform, float DeltaSeconds)
{
    if (!Guitar || !HandTarget)
    {
        return;
    }

    TArray<FVector> Contacts[4];
    FVector ContactSum = FVector::ZeroVector;
    int32 PressingFingerCount = 0;
    for (int32 Finger = 1; Finger <= 4; ++Finger)
    {
        Contacts[Finger - 1] = Guitar->GetFingerTargets(Finger);
        const TArray<FVector>& FingerContacts = Contacts[Finger - 1];
        if (!FingerContacts.IsEmpty())
        {
            FVector Center = FVector::ZeroVector;
            for (const FVector& Contact : FingerContacts)
            {
                Center += Contact;
            }
            // 每根手指贡献一个中心，横按食指不会因接触点多而占更大权重。
            ContactSum += Center / FingerContacts.Num();
            ++PressingFingerCount;
        }
    }

    UpdateFingerTarget(Contacts[0], IndexTarget, IndexWeight, MeshTransform, DeltaSeconds);
    UpdateFingerTarget(Contacts[1], MiddleTarget, MiddleWeight, MeshTransform, DeltaSeconds);
    UpdateFingerTarget(Contacts[2], RingTarget, RingWeight, MeshTransform, DeltaSeconds);
    UpdateFingerTarget(Contacts[3], LittleTarget, LittleWeight, MeshTransform, DeltaSeconds);

    FVector DesiredOffset = FVector::ZeroVector;
    if (PressingFingerCount > 0)
    {
        // 当前蓝图以 Am 手型摆放手腕；其他和弦根据按弦中心相对 Am 的偏移移动。
        const FVector Reference = (Guitar->GetFretPosition(1, 1) +
            Guitar->GetFretPosition(3, 2) + Guitar->GetFretPosition(2, 2)) / 3.0f;
        DesiredOffset = MeshTransform.InverseTransformVector(ContactSum / PressingFingerCount - Reference);
    }

    const bool bBarre = Contacts[0].Num() > 1;
    BarreWeight = FMath::FInterpTo(BarreWeight, bBarre ? 1.0f : 0.0f, DeltaSeconds, 15.0f);
    // 退出横按时保留上一帧姿势，随权重渐退；普通指尖 IK 会重新接管。
    const FVector BarreOffset = bBarre ?
        BarrePose.Calculate(Guitar, Contacts[0], MeshTransform, LeftHandTarget) : HandOffset;
    DesiredOffset = FMath::Lerp(DesiredOffset, BarreOffset, BarreWeight);
    HandOffset = FMath::VInterpTo(HandOffset, DesiredOffset, DeltaSeconds, 10.0f);
    LeftHandTarget.AddToTranslation(HandOffset);

    if (bBarre)
    {
        BarrePose.GetLocalTransforms(LeftHandTarget, BarreRoot, BarreMiddle, BarreEnd);
    }
}

void UGuitarPerformerAnimInstance::UpdateRightHand(const FTransform& MeshTransform, float DeltaSeconds)
{
    if (!Guitar || !RightHandTargetComponent)
    {
        return;
    }

    // 拨弦组件给出世界空间指尖位置；这里转换空间，骨骼求解由 Control Rig 完成。
    PickThumbTarget.SetTranslation(MeshTransform.InverseTransformPosition(Picking->GetFingerPosition(0)));
    PickIndexTarget.SetTranslation(MeshTransform.InverseTransformPosition(Picking->GetFingerPosition(1)));
    PickMiddleTarget.SetTranslation(MeshTransform.InverseTransformPosition(Picking->GetFingerPosition(2)));
    PickRingTarget.SetTranslation(MeshTransform.InverseTransformPosition(Picking->GetFingerPosition(3)));
    PickingWeight = FMath::FInterpTo(PickingWeight, 1.0f, DeltaSeconds, 12.0f);
}
