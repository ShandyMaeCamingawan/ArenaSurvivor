using UnrealBuildTool;

public class ArenaSurvivorTarget : TargetRules
{
	public ArenaSurvivorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("ArenaSurvivor");
	}
}
