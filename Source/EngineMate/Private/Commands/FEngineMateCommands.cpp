
#include "EngineMate/Public/Commands/FEngineMateCommands.h"
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"


#define LOCTEXT_NAMESPACE "FEngineMateCommands"

void FEngineMateCommands::RegisterCommands()
{
	UI_COMMAND(
		OpenPerformanceMonitorWindow,
		"Performance Monitor",
		"Performance Monitor",
		EUserInterfaceActionType::Button,
		FInputChord()
	);

	UI_COMMAND(
		OpenLevelAssistWindow, 
		"Level Assist",
		"Level Design Assist",
		EUserInterfaceActionType::Button,
		FInputChord()
	);
}

#undef LOCTEXT_NAMESPACE