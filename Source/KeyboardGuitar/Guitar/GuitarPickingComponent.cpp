#include "GuitarPickingComponent.h"
#include "GuitarInstrument.h"

namespace
{
    // 弦索引 0～5 是第一弦到第六弦；手指索引 0～3 是拇、食、中、无名指。
    constexpr int32 FingerForString[] = { 3, 2, 1, 0, 0, 0 };
    constexpr float ContactPositions[] = { 0.17f, 0.24f, 0.21f, 0.18f };

    // 位置沿琴桥到琴枕的方向取 0～1；以下距离是世界空间中的厘米。
    constexpr float ContactHeight = 0.4f;
    constexpr float ReadyHeight = 0.9f;
    constexpr float PullHeight = 1.0f;
    constexpr float PullDistance = 0.9f;
    constexpr float PullTimeRatio = 0.25f;
}

UGuitarPickingComponent::UGuitarPickingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGuitarPickingComponent::BeginPlay()
{
    Super::BeginPlay();
    StopAll();
}

void UGuitarPickingComponent::StartPick(int32 StringIndex, int32 Fret)
{
    const int32 Finger = FingerForString[StringIndex];
    // 重复拨弦或拇指切换低音弦时，从当前指尖位置开始，避免突然跳位。
    const FVector Start = GetFingerPosition(Finger);
    FPickState& State = States[StringIndex];
    State.ElapsedTime = 0.0f;
    State.Fret = Fret;
    State.bActive = true;
    State.bSoundPending = true;
    State.StartPosition = Start;
    FingerStrings[Finger] = StringIndex;
    SetComponentTickEnabled(true);
}

void UGuitarPickingComponent::CancelString(int32 StringIndex)
{
    FPickState& State = States[StringIndex];
    State.bActive = false;
    State.bSoundPending = false;
}

void UGuitarPickingComponent::StopAll()
{
    for (int32 StringIndex = 0; StringIndex < 6; ++StringIndex)
    {
        CancelString(StringIndex);
    }
    SetComponentTickEnabled(false);
}

void UGuitarPickingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AGuitarInstrument* Guitar = Cast<AGuitarInstrument>(GetOwner());
    bool bAnyActive = false;
    for (int32 StringIndex = 0; StringIndex < 6; ++StringIndex)
    {
        FPickState& State = States[StringIndex];
        if (!State.bActive)
        {
            continue;
        }

        State.ElapsedTime += DeltaTime;
        // 触弦只发声一次；保存按键时的品位，准备期间换和弦不会改变本次音高。
        if (State.bSoundPending && State.ElapsedTime >= ContactTime)
        {
            State.bSoundPending = false;
            Guitar->SoundString(StringIndex, State.Fret);
        }
        State.bActive = State.ElapsedTime < ContactTime + RecoveryTime;
        bAnyActive = bAnyActive || State.bActive;
    }
    SetComponentTickEnabled(bAnyActive);
}

FVector UGuitarPickingComponent::GetFaceNormal() const
{
    const AGuitarInstrument* Guitar = Cast<AGuitarInstrument>(GetOwner());
    const FVector AlongNeck = Guitar->GetPickPosition(0, 1.0f) - Guitar->GetPickPosition(0, 0.0f);
    const FVector AcrossStrings = Guitar->GetPickPosition(0, 0.24f) - Guitar->GetPickPosition(5, 0.24f);
    // 琴颈方向与粗弦到细弦方向的叉积，给出指向琴面外侧的法线。
    return FVector::CrossProduct(AlongNeck, AcrossStrings).GetSafeNormal();
}

FVector UGuitarPickingComponent::GetFingerPosition(int32 Finger) const
{
    const AGuitarInstrument* Guitar = Cast<AGuitarInstrument>(GetOwner());
    const int32 StringIndex = FingerStrings[Finger];
    const float PositionAlongString = ContactPositions[Finger];
    const FVector Contact = Guitar->GetPickPosition(StringIndex, PositionAlongString);
    const FVector Normal = GetFaceNormal();
    const FVector AcrossStrings = (Guitar->GetPickPosition(0, PositionAlongString) -
        Guitar->GetPickPosition(5, PositionAlongString)).GetSafeNormal();
    const FVector PullDirection = Finger == 0 ? AcrossStrings : -AcrossStrings;
    const FPickState& State = States[StringIndex];
    const float PullTime = RecoveryTime * PullTimeRatio;

    if (!State.bActive)
    {
        return Contact + Normal * ReadyHeight;
    }

    // 准备阶段：从当前位置靠近弦面，结束时指腹接触琴弦。
    if (State.ElapsedTime < ContactTime)
    {
        const float Alpha = FMath::SmoothStep(0.0f, 1.0f, State.ElapsedTime / ContactTime);
        return FMath::Lerp(State.StartPosition, Contact + Normal * ContactHeight, Alpha);
    }

    // 拨出阶段：抬起指尖并横向带弦；拇指与其他手指的方向相反。
    if (State.ElapsedTime < ContactTime + PullTime)
    {
        const float Alpha = (State.ElapsedTime - ContactTime) / PullTime;
        return Contact + Normal * FMath::Lerp(ContactHeight, PullHeight, Alpha) +
            PullDirection * (PullDistance * Alpha);
    }

    // 恢复阶段：从拨出位置平滑回到弦面上方的准备位置。
    const float Alpha = FMath::Clamp((State.ElapsedTime - ContactTime - PullTime) /
        (RecoveryTime - PullTime), 0.0f, 1.0f);
    const float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
    return Contact + Normal * FMath::Lerp(PullHeight, ReadyHeight, SmoothAlpha) +
        PullDirection * (PullDistance * (1.0f - SmoothAlpha));
}
