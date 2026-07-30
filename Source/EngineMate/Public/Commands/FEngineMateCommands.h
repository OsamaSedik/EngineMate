#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Style/PluginStyle.h"
#include "Styling/AppStyle.h"


class FEngineMateCommands : public TCommands<FEngineMateCommands>
{
	
public:
	FEngineMateCommands()
	: TCommands<FEngineMateCommands>(
		TEXT("EngineMate"),
		NSLOCTEXT("Contexts", "EngineMate", "Engine Mate Plugin"),
		NAME_None,
		FEngineMateStyle::GetStyleSetName() 
	)
	{
		
	}
	
	virtual void RegisterCommands() override;
	TSharedPtr<FUICommandInfo> OpenPerformanceMonitorWindow;
	TSharedPtr<FUICommandInfo> OpenLevelAssistWindow;
};
