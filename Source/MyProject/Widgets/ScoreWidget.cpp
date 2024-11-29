// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreWidget.h"

#include "MyProject/MyProjectCharacter.h"


void UScoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (AMyProjectCharacter* Character = Cast<AMyProjectCharacter>(GetOwningPlayerPawn()))
	{
		BP_OnCharacterInitialized(Character);
	}
}
