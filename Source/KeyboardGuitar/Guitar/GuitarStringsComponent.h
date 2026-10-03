#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuitarStringsComponent.generated.h"

class UInstancedStaticMeshComponent;
class USceneComponent;

/** 在蓝图中配置叠加琴弦的形状和视觉振动参数。 */
USTRUCT(BlueprintType)
struct FGuitarStringSettings
{
    GENERATED_BODY()

    /** 每根弦的圆柱段数量；段数越多，弯曲越平滑。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "4", ClampMax = "64"))
    int32 SegmentsPerString = 24;

    /** 第一弦半径，单位：琴弦网格局部坐标中的厘米。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.001"))
    float FirstStringRadius = 0.025f;

    /** 第六弦半径；中间四根弦的半径通过插值计算。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.001"))
    float SixthStringRadius = 0.055f;

    /** 最大横向振幅，单位：琴弦网格局部坐标中的厘米。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float Amplitude = 0.15f;

    /** 视觉振动频率，单位：次/秒；声音音高仍由音频采样决定。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.1"))
    float Frequency = 8.0f;

    /** 指数衰减系数，单位：1/秒；数值越大，振动停止越快。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.1"))
    float Decay = 3.0f;
};

/** 生成圆柱段并更新视觉振动，也提供静止琴弦上的品位位置。 */
UCLASS(ClassGroup = (Guitar))
class KEYBOARDGUITAR_API UGuitarStringsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGuitarStringsComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guitar|Strings")
    FGuitarStringSettings Settings;

    /** 读取四个定位点、重置振动状态并生成琴弦；查询位置和拨弦前调用。 */
    void Initialize(UInstancedStaticMeshComponent* Mesh,
        USceneComponent* FirstNut, USceneComponent* FirstBridge,
        USceneComponent* SixthNut, USceneComponent* SixthBridge);

    /** 弦索引 0～5；记录拨弦时的非负品位，重新启动该弦的振动。 */
    void Pluck(int32 StringIndex, int32 Fret);

    /** 停止一根弦的振动并立即恢复平直。 */
    void StopString(int32 StringIndex);

    /** 停止所有弦的振动并关闭 Tick。 */
    void StopAll();

    /** 返回静止弦上品格内的世界位置；Fret 为 0 时返回琴枕位置。 */
    FVector GetFretPosition(int32 StringIndex, int32 Fret) const;

    /** 静止琴弦的世界位置，0 为琴桥，1 为琴枕。 */
    FVector GetPickPosition(int32 StringIndex, float PositionAlongString) const;

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    /** 每根弦单独保存状态，互不影响。 */
    struct FStringState
    {
        float Strength = 0.0f; // 剩余振动强度，拨弦时为 1，静止时为 0。
        float Time = 0.0f;     // 距离本次拨弦经过的秒数。
        int32 Fret = 0;        // 拨弦瞬间的品位，后续切换和弦不会改变它。
    };

    UPROPERTY(Transient)
    TObjectPtr<UInstancedStaticMeshComponent> StringMeshes;

    FStringState States[6];

    /** 端点和振动方向均使用 StringMeshes 的局部坐标。 */
    FVector NutPositions[6];
    FVector BridgePositions[6];
    FVector VibrationDirection = FVector::ZeroVector;

    /** 按弦索引依次创建圆柱段，再摆放到静止位置。 */
    void BuildStrings();

    /** 根据当前振动状态，更新各圆柱段的位置、旋转和缩放。 */
    void UpdateStrings();

    /** 静止弦上的局部位置，0 为琴桥，1 为琴枕；供所有位置查询共用。 */
    FVector GetRestPoint(int32 StringIndex, float PositionAlongString) const;

    /** 在静止位置上叠加振动，用于摆放圆柱段。 */
    FVector GetStringPoint(int32 StringIndex, float PositionAlongString) const;
};
