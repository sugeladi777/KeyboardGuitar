// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// 游戏构建目标，用于编译游戏程序。
public class KeyboardGuitarTarget : TargetRules
{
    public KeyboardGuitarTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("KeyboardGuitar");
    }
}
