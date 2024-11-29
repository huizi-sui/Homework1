// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreWidget.generated.h"

class AMyProjectCharacter;
/**
 * 
 */
UCLASS()
class MYPROJECT_API UScoreWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Owning Character Initialized"))
	void BP_OnCharacterInitialized(AMyProjectCharacter* OwningCharacter);
};

