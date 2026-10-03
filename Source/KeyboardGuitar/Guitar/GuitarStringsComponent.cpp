#include "GuitarStringsComponent.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"

UGuitarStringsComponent::UGuitarStringsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGuitarStringsComponent::Initialize(UInstancedStaticMeshComponent* Mesh,
    USceneComponent* FirstNut, USceneComponent* FirstBridge,
    USceneComponent* SixthNut, USceneComponent* SixthBridge)
{
    StringMeshes = Mesh;

    // 定位点来自世界空间；琴弦几何统一保存在 StringMeshes 局部空间。
    const FTransform MeshTransform = Mesh->GetComponentTransform();
    const FVector FirstNutPosition = MeshTransform.InverseTransformPosition(FirstNut->GetComponentLocation());
    const FVector SixthNutPosition = MeshTransform.InverseTransformPosition(SixthNut->GetComponentLocation());
    const FVector FirstBridgePosition = MeshTransform.InverseTransformPosition(FirstBridge->GetComponentLocation());
    const FVector SixthBridgePosition = MeshTransform.InverseTransformPosition(SixthBridge->GetComponentLocation());

    VibrationDirection = (SixthNutPosition - FirstNutPosition).GetSafeNormal();
    for (int32 StringIndex = 0; StringIndex < 6; ++StringIndex)
    {
        // 第一弦取 0，第六弦取 1；中间四根弦按等间隔插值。
        const float Alpha = StringIndex / 5.0f;
        NutPositions[StringIndex] = FMath::Lerp(FirstNutPosition, SixthNutPosition, Alpha);
        BridgePositions[StringIndex] = FMath::Lerp(FirstBridgePosition, SixthBridgePosition, Alpha);
        States[StringIndex] = FStringState();
    }

    SetComponentTickEnabled(false);
    BuildStrings();
}

void UGuitarStringsComponent::Pluck(int32 StringIndex, int32 Fret)
{
    FStringState& State = States[StringIndex];
    State.Strength = 1.0f;
    State.Time = 0.0f;
    State.Fret = Fret;
    SetComponentTickEnabled(true);
}

void UGuitarStringsComponent::StopString(int32 StringIndex)
{
    States[StringIndex] = FStringState();
    UpdateStrings();
}

void UGuitarStringsComponent::StopAll()
{
    for (FStringState& State : States)
    {
        State = FStringState();
    }
    UpdateStrings();
    SetComponentTickEnabled(false);
}

void UGuitarStringsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    bool bStillVibrating = false;
    for (FStringState& State : States)
    {
        if (State.Strength <= 0.0f)
        {
            continue;
        }

        State.Time += DeltaTime;
        // 指数衰减使用经过的秒数，使振动持续时间基本不受帧率影响。
        State.Strength *= FMath::Exp(-Settings.Decay * DeltaTime);
        if (State.Strength < 0.001f)
        {
            State.Strength = 0.0f;
        }
        bStillVibrating = bStillVibrating || State.Strength > 0.0f;
    }

    // 停止前仍更新最后一帧，让所有圆柱段恢复到平直位置。
    UpdateStrings();
    SetComponentTickEnabled(bStillVibrating);
}

FVector UGuitarStringsComponent::GetFretPosition(int32 StringIndex, int32 Fret) const
{
    FVector LocalPosition = NutPositions[StringIndex];
    if (Fret > 0)
    {
        // 十二平均律：每升一品，有效弦长乘以 2^(-1/12)。
        const float CurrentRatio = FMath::Pow(2.0f, -Fret / 12.0f);
        const float PreviousRatio = FMath::Pow(2.0f, -(Fret - 1) / 12.0f);
        const FVector CurrentFret = GetRestPoint(StringIndex, CurrentRatio);
        const FVector PreviousFret = GetRestPoint(StringIndex, PreviousRatio);
        // 在品格内走 75%，把指腹目标放在靠近当前品丝的位置。
        LocalPosition = FMath::Lerp(PreviousFret, CurrentFret, 0.75f);
    }
    return StringMeshes->GetComponentTransform().TransformPosition(LocalPosition);
}

FVector UGuitarStringsComponent::GetPickPosition(int32 StringIndex, float PositionAlongString) const
{
    return StringMeshes->GetComponentTransform().TransformPosition(GetRestPoint(StringIndex, PositionAlongString));
}

void UGuitarStringsComponent::BuildStrings()
{
    StringMeshes->ClearInstances();
    // 实例按弦号连续排列，实际形状统一交给 UpdateStrings 设置。
    for (int32 Index = 0; Index < 6 * Settings.SegmentsPerString; ++Index)
    {
        StringMeshes->AddInstance(FTransform::Identity);
    }
    UpdateStrings();
}

FVector UGuitarStringsComponent::GetRestPoint(int32 StringIndex, float PositionAlongString) const
{
    return FMath::Lerp(BridgePositions[StringIndex], NutPositions[StringIndex], PositionAlongString);
}

FVector UGuitarStringsComponent::GetStringPoint(int32 StringIndex, float PositionAlongString) const
{
    const FVector RestPosition = GetRestPoint(StringIndex, PositionAlongString);
    const FStringState& State = States[StringIndex];
    const float VibratingLength = FMath::Pow(2.0f, -State.Fret / 12.0f);
    if (State.Strength <= 0.0f || PositionAlongString >= VibratingLength)
    {
        return RestPosition;
    }

    // 只让琴桥到按弦品位之间振动，按弦点到琴枕之间保持静止。
    const float Position = PositionAlongString / VibratingLength;
    const float Shape = FMath::Sin(PI * Position); // 两端固定，中间振幅最大。
    const float Wave = FMath::Sin(2.0f * PI * Settings.Frequency * State.Time); // 随时间往返摆动。
    const float Offset = Settings.Amplitude * State.Strength * Shape * Wave;
    return RestPosition + VibrationDirection * Offset;
}

void UGuitarStringsComponent::UpdateStrings()
{
    for (int32 StringIndex = 0; StringIndex < 6; ++StringIndex)
    {
        const float Radius = FMath::Lerp(Settings.FirstStringRadius, Settings.SixthStringRadius, StringIndex / 5.0f);
        for (int32 SegmentIndex = 0; SegmentIndex < Settings.SegmentsPerString; ++SegmentIndex)
        {
            const float StartAlpha = float(SegmentIndex) / Settings.SegmentsPerString;
            const float EndAlpha = float(SegmentIndex + 1) / Settings.SegmentsPerString;
            const FVector Start = GetStringPoint(StringIndex, StartAlpha);
            const FVector End = GetStringPoint(StringIndex, EndAlpha);
            const FVector Direction = End - Start;

            // UE 基础 Cylinder 沿 Z 轴，半径 50 cm、长度 100 cm；按目标段缩放。
            const FTransform Transform(
                FRotationMatrix::MakeFromZ(Direction).ToQuat(),
                (Start + End) * 0.5f,
                FVector(Radius / 50.0f, Radius / 50.0f, Direction.Size() / 100.0f));
            const int32 InstanceIndex = StringIndex * Settings.SegmentsPerString + SegmentIndex;
            StringMeshes->UpdateInstanceTransform(InstanceIndex, Transform, false, false, true);
        }
    }
    // 各段全部更新后只通知一次渲染，避免逐段刷新。
    StringMeshes->MarkRenderStateDirty();
}
