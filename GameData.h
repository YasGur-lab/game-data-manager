// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonHelper.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include "Components/RichTextBlock.h"
#include "Sound/SoundBase.h"
#include "GameData.generated.h"

UCLASS()
class COLDWARPROJECT_API UGameData : public UObject
{
	GENERATED_BODY()

public:
	void GameData();
	UPROPERTY()
		UJsonHelper* m_JsonHelper;

	//-----------------------------------\\
	//--                               --\\
	//--       INSTRUCTIONS DATA       --\\
	//--                               --\\
	//-----------------------------------\\

	FInstructionGameData LoadInstructionsData(UWorld* World, FString path, TArray<USoundBase*> NarrativeSounds);

	//-----------------------------------\\
	//--                               --\\
	//--      AUTOMATED TOUR DATA      --\\
	//--                               --\\
	//-----------------------------------\\

	FCheckpointsGameData LoadCheckpointsData(UWorld* World, FString path, TArray<USoundBase*> NarrativeSounds, TArray<AActor*> CPActors);
	FLearnMoreGameData PopulateLearnMoreUI(FString JSONpath, int CurrentActorIndex, TArray<USoundBase*> NarrativeSounds, TArray<UTexture2D*> Images);
	TArray<UProgressBar*> LoadLearnMoreProgressBar(UHorizontalBox* progressBarsBox, FProgressBarStyle progressBarStyle, int numberOfLearnMoreOptions) const;

	//-----------------------------------\\
	//--                               --\\
	//--        RADAR GAME DATA        --\\
	//--                               --\\
	//-----------------------------------\\

	FQuizQuestions LoadQuizQuestions() const;
	FTilesGameData PopulateQuizUI(TArray<USoundBase*> NarrativeSounds, const FQuizQuestions& QuizQuestions, int32 CurrentQuestionIndex);

	//-----------------------------------\\
	//--                               --\\
	//--            GETTERS            --\\
	//--                               --\\
	//-----------------------------------\\

	static AActor* GetActorByName(const FString& ActorName, TArray<AActor*> CPActors, UWorld* World, bool& success, FString& infoMessage);
	TArray<USoundBase*> GetSoundByName(const TArray<FString> SoundNames, TArray<USoundBase*> NarrativeSounds);
	TArray<UTexture2D*> GetImageByName(const TArray<FString> ImageNames, TArray<UTexture2D*> Images);
	static Instructions StringToInstructions(const FString& InstructionType);
};
