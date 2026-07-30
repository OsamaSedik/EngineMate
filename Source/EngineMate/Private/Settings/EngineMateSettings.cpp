// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/EngineMateSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

FName UEngineMateSettings::GetCategoryName() const
{
	return FName("Plugins");
}

#if WITH_EDITOR
void UEngineMateSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property != nullptr)
	{
		FNotificationInfo Info(FText::FromString("Engine Mate settings changed. Restart now To Apply Changes"));
		Info.ExpireDuration = 7.0f;
		Info.bUseThrobber = true;
		Info.bFireAndForget = false; // Keep it visible so they can click it
    
		// Add the "Restart" Hyperlink
		Info.Hyperlink = FSimpleDelegate::CreateLambda([]()
		{
			// This command triggers a standard editor restart (asks to save dirty assets first)
			FUnrealEdMisc::Get().RestartEditor(false);
		});
		Info.HyperlinkText = FText::FromString("Restart Now");

		FSlateNotificationManager::Get().AddNotification(Info);
	}
}
#endif