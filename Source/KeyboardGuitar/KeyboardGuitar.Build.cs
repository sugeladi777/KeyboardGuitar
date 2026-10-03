// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// 运行时模块依赖：基础对象、游戏组件、音频与 Enhanced Input。
public class KeyboardGuitar : ModuleRules
{
    public KeyboardGuitar(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 按职责分目录后，允许跨目录引用本模块的头文件。
        PrivateIncludePaths.Add(ModuleDirectory);
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput"
        });
    }
}
