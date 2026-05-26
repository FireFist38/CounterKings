// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class LowPoly : ModuleRules
{
	public LowPoly(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// Use the module header as the private PCH to reduce repeated header parsing of common engine headers.
		PrivatePCHHeaderFile = "Public/LowPoly.h";

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG", "Niagara" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
