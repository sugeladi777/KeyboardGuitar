#include "GuitarInstrument.h"
#include "GuitarAudioComponent.h"
#include "GuitarChordComponent.h"
#include "Data/GuitarData.h"
#include "GuitarStringsComponent.h"
#include "GuitarFretGuideComponent.h"
#include "GuitarPickingComponent.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AGuitarInstrument::AGuitarInstrument()
{
    PrimaryActorTick.bCanEverTick = false;

    CreateSceneComponents();
    CreateGameplayComponents();
}

void AGuitarInstrument::CreateSceneComponents()
{
    // 定位点和网格共用 InstrumentFrame，整体调整吉他时保持相互对齐。
    USceneComponent* GuitarRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GuitarRoot"));
    SetRootComponent(GuitarRoot);
    InstrumentFrame = CreateDefaultSubobject<USceneComponent>(TEXT("InstrumentFrame"));
    InstrumentFrame->SetupAttachment(GuitarRoot);

    GuitarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuitarMesh"));
    GuitarMesh->SetupAttachment(InstrumentFrame);
    GuitarMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    String1Nut = CreateDefaultSubobject<USceneComponent>(TEXT("String1Nut"));
    String1Nut->SetupAttachment(InstrumentFrame);
    String1Bridge = CreateDefaultSubobject<USceneComponent>(TEXT("String1Bridge"));
    String1Bridge->SetupAttachment(InstrumentFrame);
    String6Nut = CreateDefaultSubobject<USceneComponent>(TEXT("String6Nut"));
    String6Nut->SetupAttachment(InstrumentFrame);
    String6Bridge = CreateDefaultSubobject<USceneComponent>(TEXT("String6Bridge"));
    String6Bridge->SetupAttachment(InstrumentFrame);

    StringMeshes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StringMeshes"));
    StringMeshes->SetupAttachment(InstrumentFrame);
    StringMeshes->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StringMeshes->SetCastShadow(false);
}

void AGuitarInstrument::CreateGameplayComponents()
{
    Chords = CreateDefaultSubobject<UGuitarChordComponent>(TEXT("Chords"));
    Audio = CreateDefaultSubobject<UGuitarAudioComponent>(TEXT("Audio"));
    Strings = CreateDefaultSubobject<UGuitarStringsComponent>(TEXT("Strings"));
    FretGuide = CreateDefaultSubobject<UGuitarFretGuideComponent>(TEXT("FretGuide"));
    FretGuide->SetupAttachment(InstrumentFrame);
    Picking = CreateDefaultSubobject<UGuitarPickingComponent>(TEXT("Picking"));
}

void AGuitarInstrument::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    // 编辑器中调整配置时重新生成；先准备琴弦坐标，再更新提示。
    InitializeInstrument();
}

void AGuitarInstrument::BeginPlay()
{
    Super::BeginPlay();
    // 游戏开始时按最终组件变换重新读取定位点。
    InitializeInstrument();
}

void AGuitarInstrument::InitializeInstrument()
{
    Strings->Initialize(StringMeshes, String1Nut, String1Bridge, String6Nut, String6Bridge);
    FretGuide->Refresh(this);
}

void AGuitarInstrument::PlayString(int32 StringIndex)
{
    // 重复拨弦先停止旧音；静音弦同样需要停止旧音和振动。
    Audio->StopString(StringIndex);
    Strings->StopString(StringIndex);
    const int32 Fret = Chords->GetFretForString(StringIndex, GuitarData);
    if (Fret < 0)
    {
        Picking->CancelString(StringIndex);
        return;
    }
    Picking->StartPick(StringIndex, Fret);
}

void AGuitarInstrument::SoundString(int32 StringIndex, int32 Fret)
{
    // 每升一个品位升高一个半音，对应 MIDI 音高编号增加 1。
    const int32 MidiNote = GuitarData->OpenStringNotes[StringIndex] + Fret;
    Audio->PlayNote(StringIndex, MidiNote, GuitarData);
    Strings->Pluck(StringIndex, Fret);
}

void AGuitarInstrument::StopStrings()
{
    Picking->StopAll();
    Audio->StopAll();
    Strings->StopAll();
}

FVector AGuitarInstrument::GetPickPosition(int32 StringIndex, float PositionAlongString) const
{
    return Strings->GetPickPosition(StringIndex, PositionAlongString);
}

void AGuitarInstrument::HoldChord(int32 ChordIndex)
{
    Chords->HoldChord(ChordIndex);
    FretGuide->Refresh(this);
}

void AGuitarInstrument::ReleaseChord(int32 ChordIndex)
{
    Chords->ReleaseChord(ChordIndex);
    FretGuide->Refresh(this);
}

int32 AGuitarInstrument::GetFretForString(int32 StringIndex) const
{
    return Chords->GetFretForString(StringIndex, GuitarData);
}

FVector AGuitarInstrument::GetFretPosition(int32 StringIndex, int32 Fret) const
{
    return Strings->GetFretPosition(StringIndex, Fret);
}

bool AGuitarInstrument::IsHoldingChord() const
{
    return Chords->IsHoldingChord();
}

bool AGuitarInstrument::GetFingerTarget(int32 Finger, FVector& TargetPosition) const
{
    // 单点查询复用完整接触点查询；横按时取细弦一侧的第一个点。
    const TArray<FVector> Positions = GetFingerTargets(Finger);
    TargetPosition = Positions.IsEmpty() ? FVector::ZeroVector : Positions[0];
    return !Positions.IsEmpty();
}

TArray<FVector> AGuitarInstrument::GetFingerTargets(int32 Finger) const
{
    TArray<FVector> Positions;
    if (Finger < 1 || Finger > 4)
    {
        return Positions;
    }
    for (int32 StringIndex = 0; StringIndex < 6; ++StringIndex)
    {
        if (Chords->GetFingerForString(StringIndex, GuitarData) == Finger)
        {
            Positions.Add(GetFretPosition(StringIndex, GetFretForString(StringIndex)));
        }
    }
    return Positions;
}
