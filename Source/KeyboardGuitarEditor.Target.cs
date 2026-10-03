// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// 编辑器构建目标，用于在 Unreal Editor 中加载游戏模块。
public class KeyboardGuitarEditorTarget : TargetRules
{
    public KeyboardGuitarEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("KeyboardGuitar");
    }
}
