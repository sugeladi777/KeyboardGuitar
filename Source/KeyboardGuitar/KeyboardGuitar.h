// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// 阅读入口：Data 配置演奏数据，Input 接收输入，Guitar 协调演奏，Animation 生成姿势目标。
// 拨弦流程：PlayerController -> GuitarInstrument -> Picking -> SoundString -> Audio / Strings。
// 姿势流程：GuitarInstrument / Picking -> PerformerAnimInstance -> 动画蓝图 -> Control Rig。
