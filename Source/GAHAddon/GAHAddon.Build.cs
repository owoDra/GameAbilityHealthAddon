// Copyright (C) 2023 owoDra

using UnrealBuildTool;

public class GAHAddon : ModuleRules
{
	public GAHAddon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory,
                ModuleDirectory + "/GAHAddon",
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "ModularGameplay",
                "GameplayTags",
                "GameplayAbilities",
                "NetCore",
                "GFCore",
                "UMG",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "GAExt",
            }
        );

        SetupIrisSupport(Target);
    }
}
