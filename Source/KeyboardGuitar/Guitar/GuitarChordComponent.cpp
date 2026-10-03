#include "GuitarChordComponent.h"
#include "Data/GuitarData.h"

UGuitarChordComponent::UGuitarChordComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGuitarChordComponent::HoldChord(int32 ChordIndex)
{
    // 先移除再追加，避免重复记录，并让本次按下的和弦成为当前和弦。
    HeldChordIndices.Remove(ChordIndex);
    HeldChordIndices.Add(ChordIndex);
}

void UGuitarChordComponent::ReleaseChord(int32 ChordIndex)
{
    HeldChordIndices.Remove(ChordIndex);
}

int32 UGuitarChordComponent::GetFretForString(int32 StringIndex, const UGuitarData* Data) const
{
    // 没有按住和弦时，六根弦都使用空弦。
    if (HeldChordIndices.IsEmpty())
    {
        return 0;
    }
    const int32 ChordIndex = HeldChordIndices.Last();
    const TArray<int32>& Frets = Data->Chords[ChordIndex].Frets;
    return Frets[StringIndex];
}

bool UGuitarChordComponent::IsHoldingChord() const
{
    return !HeldChordIndices.IsEmpty();
}

int32 UGuitarChordComponent::GetFingerForString(int32 StringIndex, const UGuitarData* Data) const
{
    if (HeldChordIndices.IsEmpty())
    {
        return 0;
    }
    const int32 ChordIndex = HeldChordIndices.Last();
    return Data->Chords[ChordIndex].Fingers[StringIndex];
}