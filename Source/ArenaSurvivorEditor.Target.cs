using UnrealBuildTool;

public class ArenaSurvivorEditorTarget : TargetRules
{
	public ArenaSurvivorEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("ArenaSurvivor");
	}
}
