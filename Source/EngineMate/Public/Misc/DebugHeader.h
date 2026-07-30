#pragma once
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"


namespace DebugHelper
{
	inline void PrintDebugScreen(const FString& Message,const FColor& Color,const float& DisplayTime)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,DisplayTime,Color,Message);
		}
	}
	
	inline void PrintDebugLog(const FString& Message)
	{
		UE_LOG(LogTemp,Warning,TEXT("%s"),*Message);
	}
	
	inline EAppReturnType::Type ShowMessageDialog(const EAppMsgType::Type MsgType, const FString& Message, const bool bShowMessageAsWarning =true)
	{
		if (bShowMessageAsWarning)
		{
			 return FMessageDialog::Open(MsgType,FText::FromString(Message),FText::FromString(TEXT("Warning")));
		}
		return FMessageDialog::Open(MsgType,FText::FromString(Message));
	}
	
	inline void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo = FNotificationInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeInDuration = 10.0f;
		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
}
