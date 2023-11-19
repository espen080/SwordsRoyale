// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SwordsRoyale : ModuleRules
{
	public SwordsRoyale(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });
	}
}
