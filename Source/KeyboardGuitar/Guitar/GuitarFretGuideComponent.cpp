#include "GuitarFretGuideComponent.h"
#include "GuitarInstrument.h"

UGuitarFretGuideComponent::UGuitarFretGuideComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetCastShadow(false);
}

void UGuitarFretGuideComponent::Refresh(const AGuitarInstrument* Guitar)
{
    // 先清除旧和弦的圆点；没有按住和弦时保持隐藏。
    ClearInstances();

    if (!Guitar->IsHoldingChord())
    {
        return;
    }

    const FTransform GuideTransform = GetComponentTransform();

    for (int32 StringIndex = 0; StringIndex < 6; ++StringIndex)
    {
        const int32 Fret = Guitar->GetFretForString(StringIndex);

        if (Fret < 0)
        {
            continue;
        }

        const FVector WorldPosition = Guitar->GetFretPosition(StringIndex, Fret);
        // AddInstance 使用本组件的局部坐标，转换后再加琴面偏移。
        const FVector LocalPosition = GuideTransform.InverseTransformPosition(WorldPosition) + SurfaceOffset;
        const float Scale = DotDiameter / 100.0f;
        const FTransform DotTransform(FQuat::Identity, LocalPosition, FVector(Scale));
        AddInstance(DotTransform);
    }
}
