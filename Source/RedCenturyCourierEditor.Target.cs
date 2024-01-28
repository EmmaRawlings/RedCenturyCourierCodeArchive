// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RedCenturyCourierEditorTarget : TargetRules
{
	public RedCenturyCourierEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.Add("RedCenturyCourier");
		ExtraModuleNames.AddRange(new string[] { "ModularNavigation" });
	}
}
