#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GuitarInstrument.generated.h"

class UGuitarAudioComponent;
class UGuitarChordComponent;
class UGuitarData;
class UGuitarStringsComponent;
class UInstancedStaticMeshComponent;
class USceneComponent;
class UStaticMeshComponent;
class UGuitarFretGuideComponent;
class UGuitarPickingComponent;

/** 演奏入口：组装组件，协调和弦、拨弦动作、声音、琴弦振动和品位提示。 */
UCLASS()
class KEYBOARDGUITAR_API AGuitarInstrument : public AActor
{
    GENERATED_BODY()

public:
    AGuitarInstrument();

    /** 拨动一根弦。弦索引 0～5 对应第一弦（最细）到第六弦（最粗）。 */
    UFUNCTION(BlueprintCallable, Category = "Guitar")
    void PlayString(int32 StringIndex);

    /** 由拨弦组件在触弦时调用；Fret 是按键时记录的品位。 */
    void SoundString(int32 StringIndex, int32 Fret);

    /** 返回静止弦上的世界位置；比例 0 为琴桥，1 为琴枕。 */
    FVector GetPickPosition(int32 StringIndex, float PositionAlongString) const;

    /** 停止拨弦动作、声音和振动，保留当前按住的和弦及提示。 */
    UFUNCTION(BlueprintCallable, Category = "Guitar")
    void StopStrings();

    /** 按住和弦并刷新提示；ChordIndex 对应 GuitarData.Chords 的数组索引。 */
    UFUNCTION(BlueprintCallable, Category = "Guitar")
    void HoldChord(int32 ChordIndex);

    /** 松开和弦；若仍按住其他和弦，恢复其中最后按下的一个。 */
    UFUNCTION(BlueprintCallable, Category = "Guitar")
    void ReleaseChord(int32 ChordIndex);

    /** 当前品位：-1 表示静音，0 表示空弦，正数表示按弦品位。 */
    UFUNCTION(BlueprintPure, Category = "Guitar")
    int32 GetFretForString(int32 StringIndex) const;

    /** 返回品格内提示点的世界位置；Fret 为 0 时返回琴枕位置。 */
    UFUNCTION(BlueprintPure, Category = "Guitar")
    FVector GetFretPosition(int32 StringIndex, int32 Fret) const;

    /** 是否按住至少一个和弦，用于决定品位提示的显隐。 */
    UFUNCTION(BlueprintPure, Category = "Guitar")
    bool IsHoldingChord() const;

    /**
     * 查询手指的按弦位置；Finger 为 1～4。
     * 返回 false 表示未参与按弦；横按时只返回细弦一侧的第一处世界位置。
     */
    UFUNCTION(BlueprintCallable, Category = "Guitar")
    bool GetFingerTarget(int32 Finger, FVector& TargetPosition) const;

    /** 返回手指的全部接触点，按第一弦到第六弦排列；多项表示横按。 */
    UFUNCTION(BlueprintCallable, Category = "Guitar")
    TArray<FVector> GetFingerTargets(int32 Finger) const;

protected:
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    /** 在蓝图中指定和弦指法、空弦音高和音频采样数据。 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guitar")
    TObjectPtr<UGuitarData> GuitarData;

    /** 模型、定位点和提示的共同父节点，用来整体调整吉他的姿态。 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar")
    TObjectPtr<USceneComponent> InstrumentFrame;

    /** 在蓝图中指定吉他模型，并调整模型自身的变换。 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar")
    TObjectPtr<UStaticMeshComponent> GuitarMesh;

    /** 第一弦的琴枕端和琴桥端；在蓝图中对齐模型的实际琴弦。 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar|Strings")
    TObjectPtr<USceneComponent> String1Nut;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar|Strings")
    TObjectPtr<USceneComponent> String1Bridge;

    /** 第六弦的两端；中间四根弦的位置由第一弦和第六弦插值得到。 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar|Strings")
    TObjectPtr<USceneComponent> String6Nut;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar|Strings")
    TObjectPtr<USceneComponent> String6Bridge;

    /** 叠加琴弦的圆柱段，在蓝图中指定 Cylinder 模型和琴弦材质。 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar|Strings")
    TObjectPtr<UInstancedStaticMeshComponent> StringMeshes;

    /** 记录和弦按住顺序，查询当前品位与左手指法。 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar")
    TObjectPtr<UGuitarChordComponent> Chords;

    /** 管理六路声音，按 MIDI 音高查找采样。 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar")
    TObjectPtr<UGuitarAudioComponent> Audio;

    /** 管理琴弦几何与振动，提供静止弦上的位置。 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar")
    TObjectPtr<UGuitarStringsComponent> Strings;

    /** 根据当前和弦重建品位圆点。 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar")
    TObjectPtr<UGuitarFretGuideComponent> FretGuide;

    /** 管理右手拨弦时间，在触弦时通知本 Actor 发声。 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guitar")
    TObjectPtr<UGuitarPickingComponent> Picking;

private:
    /** 构造函数创建组件；组件名称保持稳定，供继承蓝图保存配置。 */
    void CreateSceneComponents();
    void CreateGameplayComponents();

    /** 编辑器构造和游戏开始共用：先准备琴弦坐标，再刷新品位提示。 */
    void InitializeInstrument();
};
