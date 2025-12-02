// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BattleActionEditorTarget : TargetRules
{
	public BattleActionEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		//ExtraModuleNames.Add("BattleAction");
		ExtraModuleNames.AddRange( new string[] { "BattleAction", "EditorModule" } );
	}
}
