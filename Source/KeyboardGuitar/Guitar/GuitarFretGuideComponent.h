#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GuitarFretGuideComponent.generated.h"

class AGuitarInstrument;

/** 显示当前按住和弦的品位圆点；和弦变化时刷新，不使用 Tick。 */
UCLASS(ClassGroup = (Guitar))
class KEYBOARDGUITAR_API UGuitarFretGuideComponent : public UInstancedStaticMeshComponent
{
    GENERATED_BODY()

public:
    UGuitarFretGuideComponent();

    /** 重建当前和弦的提示；空弦在琴枕处显示，静音弦不显示。 */
    void Refresh(const AGuitarInstrument* Guitar);

    /** 圆点直径，单位：本组件局部坐标中的厘米；模型使用直径 100 cm 的基础 Sphere。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guitar|Guide", meta = (ClampMin = "0.1"))
    float DotDiameter = 0.7f;

    /** 本组件局部坐标中的偏移，单位：厘米；让圆点略高于琴面。 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guitar|Guide")
    FVector SurfaceOffset = FVector(0.0f, 0.3f, 0.0f);
};
