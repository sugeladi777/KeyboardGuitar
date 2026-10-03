#include "GuitarAudioComponent.h"
#include "Data/GuitarData.h"

#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundWave.h"

UGuitarAudioComponent::UGuitarAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGuitarAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    // 运行时创建并注册声音组件；关闭自动播放，由拨弦操作启动声音。
    for (int32 StringIndex = 0; StringIndex < 6; ++StringIndex)
    {
        UAudioComponent* Voice = NewObject<UAudioComponent>(GetOwner());
        Voice->bAutoActivate = false;
        Voice->SetupAttachment(GetOwner()->GetRootComponent());
        Voice->RegisterComponent();
        Voices.Add(Voice);
    }
}

void UGuitarAudioComponent::PlayNote(int32 StringIndex, int32 MidiNote, const UGuitarData* Data)
{
    USoundWave* Sound = Data->NoteSamples.FindRef(MidiNote).Get();
    UAudioComponent* Voice = Voices[StringIndex];
    Voice->Stop();
    Voice->SetSound(Sound);
    Voice->SetVolumeMultiplier(StringVolume);
    Voice->Play();
}

void UGuitarAudioComponent::StopString(int32 StringIndex)
{
    Voices[StringIndex]->Stop();
}

void UGuitarAudioComponent::StopAll()
{
    for (UAudioComponent* Voice : Voices)
    {
        Voice->Stop();
    }
}
