#include "GameData.h"
#include "JsonHelper.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

/*************************************
Class: UGameData
Author: Antoine Plouffe

Description: The UGameData class serves as a central hub for handling various game data
and functionalities for the application. It includes methods for loading instructions, checkpoints,
learn more content, and quiz questions from a custom JSON Reader. Additionally, it provides
functions for populating UI elements, such as progress bars and quiz options, based
on the loaded data. The class encapsulates error handling to display debug messages in case of data loading issues.
*************************************/

void UGameData::GameData()
{
	m_JsonHelper = NewObject<UJsonHelper>();
}

//-----------------------------------\\
//--                               --\\
//--       INSTRUCTIONS DATA       --\\
//--                               --\\
//-----------------------------------\\

//Read instruction data from a JSON file specified by the given path.
//It creates a structured representation of the instruction data, mapping
//instruction types to their corresponding narration keys and sound assets.
//Additionally, it handles error reporting by displaying debug messages
//if any issues occur during the data loading process.
FInstructionGameData UGameData::LoadInstructionsData(UWorld* World, FString path, TArray<USoundBase*> NarrativeSounds)
{
	bool success;
	FString message;

	FInstructionsData dataStructure = m_JsonHelper->ReadStructFromJsonFile<FInstructionsData>(path, success, message);
	FInstructionGameData instructionData;
	for (int i = 0; i < dataStructure.Data.Num(); i++)
	{
		FInstructionNarration narrationKeys;
		narrationKeys.m_TitleKey = dataStructure.Data[i].TitleCaptionKey;
		for (auto captionKey : dataStructure.Data[i].CaptionKeys)
		{
			narrationKeys.m_Keys.Add(captionKey);
		}
		narrationKeys.m_EnglishNarrationSounds = GetSoundByName(dataStructure.Data[i].EnglishNarrationSoundNames, NarrativeSounds);
		narrationKeys.m_FrenchNarrationSounds = GetSoundByName(dataStructure.Data[i].FrenchNarrationSoundNames, NarrativeSounds);
		instructionData.InstructionKeyMap.Add(StringToInstructions(dataStructure.Data[i].InstructionType), narrationKeys);

		if (!success)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, message);
		}
	}

	return instructionData;
}

//-----------------------------------\\
//--                               --\\
//--      AUTOMATED TOUR DATA      --\\
//--                               --\\
//-----------------------------------\\

//Read checkpoint data from a JSON file specified by the given path.
//It creates a structured representation of the checkpoint data, including
//actors to follow, their associated frame numbers, and narration keys with
//relevant sound assets. Additionally, the method populates the game data
//structure with information about whether a checkpoint has associated
//learn more options or quizzes. Error handling is incorporated to display
//debug messages in case of loading issues.
FCheckpointsGameData UGameData::LoadCheckpointsData(UWorld* World, FString path, TArray<USoundBase*> NarrativeSounds, TArray<AActor*> CPActors)
{
	bool success;
	FString message;

	FCheckpointsGameData gameData;
	static FCheckpointsData DataStructure = m_JsonHelper->ReadStructFromJsonFile<FCheckpointsData>(path, success, message);

	for (int i = 0; i < DataStructure.Data.Num(); i++)
	{
		AActor* actor = GetActorByName(DataStructure.Data[i].CheckpointName, CPActors, World, success, message);

		gameData.ActorsToFollow.Add(actor);
		gameData.ActorFrameMap.Add(actor, DataStructure.Data[i].CheckpointFrameNumber);

		FNarrationKeys narrationKeys;
		narrationKeys.m_TitleKey = DataStructure.Data[i].TitleCaptionKey;
		for (auto captionKey : DataStructure.Data[i].CaptionKeys)
		{
			narrationKeys.m_Keys.Add(captionKey);
		}
		narrationKeys.m_EnglishNarrationSounds = GetSoundByName(DataStructure.Data[i].EnglishNarrationSoundNames, NarrativeSounds);
		narrationKeys.m_FrenchNarrationSounds = GetSoundByName(DataStructure.Data[i].FrenchNarrationSoundNames, NarrativeSounds);
		narrationKeys.m_ShouldStopCamera = DataStructure.Data[i].ShouldStopCamera;
		narrationKeys.m_HasLearnMoreOption = DataStructure.Data[i].HasLearnMoreOption;
		narrationKeys.m_HasQuiz = DataStructure.Data[i].HasQuiz;
		narrationKeys.m_NumOfLearnMoreOptions = DataStructure.Data[i].NumOfLearnMoreOption;
		gameData.ActorKeyMap.Add(actor, narrationKeys);

		if (!success)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, message);
		}
	}

	return gameData;
}

//Read learn more data from a JSON file specified by the given path.
//It creates a structured representation of the learn more data,
//including narration sounds, images, captions, and source names.
//The method specifically filters and populates data corresponding
//to the current actor index, ensuring that it captures information
//relevant to the current checkpoint for display in the Learn More UI.
//Error handling is integrated to handle potential issues during data
//loading and display debug messages if necessary.
FLearnMoreGameData UGameData::PopulateLearnMoreUI(FString JSONpath, int CurrentActorIndex, TArray<USoundBase*> NarrativeSounds, TArray<UTexture2D*> Images)
{
	bool success;
	FString message;

	FLearnMoreGameData learnMoreGameData;
	FLearnMoreData dataStructure = m_JsonHelper->ReadStructFromJsonFile<FLearnMoreData>(JSONpath, success, message);
	
	for (auto data : dataStructure.Data)
	{
		if (data.CorrespondingCPIndex == CurrentActorIndex)
		{
			FLearnMoreNarration learnMoreNarration;
			learnMoreNarration.m_FrenchNarrationSounds = GetSoundByName(data.FrenchNarrationSoundNames, NarrativeSounds);
			learnMoreNarration.m_EnglishNarrationSounds = GetSoundByName(data.EnglishNarrationSoundNames, NarrativeSounds);
			learnMoreNarration.m_Images = GetImageByName(data.ImagesNames, Images);
			learnMoreNarration.CorrespondingCPIndex = data.CorrespondingCPIndex;
			learnMoreNarration.m_TitleKey = data.TitleCaptionKey;
			learnMoreNarration.m_Keys = data.CaptionKeys;
			if (!data.ImagesSources.IsEmpty())
			{
				learnMoreNarration.m_SourceName = data.ImagesSources[0];
			}
			learnMoreGameData.LearnMoreData.Add(learnMoreNarration);
		}
	}

	return learnMoreGameData;
}

//Dynamically creates UProgressBar instances, configures their
//appearance and layout within the provided horizontal box,
//and returns an array of these progress bars.This facilitates
//the visual representation of the user's progress in exploring
//Learn More content.
TArray<UProgressBar*> UGameData::LoadLearnMoreProgressBar(UHorizontalBox* progressBarsBox, FProgressBarStyle progressBarStyle, int numberOfLearnMoreOptions) const
{
	TArray<UProgressBar*> learnMoreProgressBars;
	for (size_t i = 0; i < numberOfLearnMoreOptions; i++)
	{
		if (UProgressBar* progressBar = NewObject<UProgressBar>())
		{
			UHorizontalBoxSlot* horizontalSlot = Cast<UHorizontalBoxSlot>(progressBarsBox->AddChild(progressBar));
			horizontalSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			horizontalSlot->SetHorizontalAlignment(HAlign_Fill);
			horizontalSlot->SetVerticalAlignment(VAlign_Fill);
			progressBar->SetPercent(0.0f);
			progressBar->SetBarFillType(EProgressBarFillType::LeftToRight);
			progressBar->SetBarFillStyle(EProgressBarFillStyle::Mask);
			progressBar->SetWidgetStyle(progressBarStyle);
			learnMoreProgressBars.Add(progressBar);
		}
	}
	return learnMoreProgressBars;
}

//-----------------------------------\\
//--                               --\\
//--        RADAR GAME DATA        --\\
//--                               --\\
//-----------------------------------\\

//Read and loading quiz questions from a JSON file.
//It utilizes the m_JsonHelper to deserialize the
//JSON data into the FQuizQuestions structure.
//In case of any issues, it displays an on-screen debug message.
//The method ultimately returns the loaded quiz data.
FQuizQuestions UGameData::LoadQuizQuestions() const
{
	bool success;
	FString message;

	const FString FilePath = FPaths::ProjectContentDir() + "/JSONFiles/AutomatedTour/quiz.json";
	FQuizQuestions quizData = m_JsonHelper->ReadStructFromJsonFile<FQuizQuestions>(FilePath, success, message);

	if (!success)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, message);
	}

	return quizData;
}

//Populates data for the quiz user interface. It takes
//information from the provided FQuizQuestions structure,
//extracts options for the current quiz question, and
//organizes them into a format suitable for the quiz UI.
//This method builds a mapping of quiz options to their
//respective narrations and sound cues, encapsulating the data needed.
FTilesGameData UGameData::PopulateQuizUI(TArray<USoundBase*> NarrativeSounds, const FQuizQuestions& QuizQuestions, int32 CurrentQuestionIndex)
{
	FTilesGameData tilesData;
	TArray<FQuizQuestionOption> options = QuizQuestions.m_Questions[CurrentQuestionIndex].QuestionOptions.Options;
	TMap<int, FLearnMoreNarration> narrationMap;
	FLearnMoreNarration narration;

	for (int i = 0; i < options.Max(); i++)
	{
		narration.m_TitleKey = options[i].OptionName;
		narration.m_Keys.Add(options[i].OptionDescription);

		TArray<FString> sounds;
		sounds.Add(QuizQuestions.m_Questions[CurrentQuestionIndex].QuestionOptions.Options[i].EnglishNarrationSound);
		narration.m_EnglishNarrationSounds = GetSoundByName(sounds, NarrativeSounds);
		sounds.Empty();
		sounds.Add(QuizQuestions.m_Questions[CurrentQuestionIndex].QuestionOptions.Options[i].FrenchNarrationSound);
		narration.m_FrenchNarrationSounds = GetSoundByName(sounds, NarrativeSounds);
		narrationMap.Add(i, narration);
	}

	tilesData.LearnMoreKeyMap = narrationMap;
	return tilesData;
}

//-----------------------------------\\
//--                               --\\
//--            GETTERS            --\\
//--                               --\\
//-----------------------------------\\

//Designed to retrieve an actor with a specific name from
//a provided array of actors (CPActors). The method also
//sets a success flag and provides an information message
//to indicate whether the actor was successfully located or not.
AActor* UGameData::GetActorByName(const FString& ActorName, TArray<AActor*> CPActors, UWorld* World, bool& success, FString& infoMessage)
{
	for (AActor* cp : CPActors)
	{
		if (cp && cp->Tags[0] == FName(ActorName))
		{
			success = true;
			infoMessage = FString("Actor Found");
			return cp;
		}
	}
	success = false;
	infoMessage = FString("Actor Not Found");;
	return nullptr;
}

//Designed to retrieve an array of sound assets based on a
//provided array of sound names (SoundNames) and an existing
//array of sound assets (NarrativeSounds)
TArray<USoundBase*> UGameData::GetSoundByName(const TArray<FString> SoundNames, TArray<USoundBase*> NarrativeSounds)
{
	TArray<USoundBase*> sounds;
	for (FString soundName : SoundNames)
	{
		for (USoundBase* Sound : NarrativeSounds)
		{
			if (Sound && Sound->GetName() == soundName && !sounds.Contains(Sound))
			{
				sounds.Add(Sound);
			}
		}
	}
	return sounds;
}

//Designed to retrieve an array of images assets based on a
//provided array of images names (ImageNames) and an existing
//array of images assets (Images)
TArray<UTexture2D*> UGameData::GetImageByName(const TArray<FString> ImageNames, TArray<UTexture2D*> Images)
{
	TArray<UTexture2D*> foundImages;

	for (FString imageName : ImageNames)
	{
		for (UTexture2D* Image : Images)
		{
			if (Image && Image->GetName() == imageName && !foundImages.Contains(Image))
			{
				foundImages.Add(Image);
			}
		}
	}
	return foundImages;
}

//Converts a given string representation of an instruction
//type (InstructionType) into its corresponding enumerated
//value from the Instructions enum.
Instructions UGameData::StringToInstructions(const FString& InstructionType)
{
	if (InstructionType == "LearnMoreProposed") return Instructions::LearnMoreProposed;
	if (InstructionType == "LearnMoreCompleted") return Instructions::LearnMoreCompleted;
	if (InstructionType == "HowToSelection") return Instructions::HowToSelection;
	if (InstructionType == "QuizProposed") return Instructions::QuizProposed;
	if (InstructionType == "LearnMoreNavigation") return Instructions::LearnMoreNavigation;
	if (InstructionType == "MiniGameQuiz_Context") return Instructions::MiniGameQuiz_Context;
	if (InstructionType == "MiniGameQuiz_QuestionInstruction") return Instructions::MiniGameQuiz_QuestionInstruction;
	if (InstructionType == "Inactivity_Instruction") return Instructions::Inactivty_Instruction;
	return Instructions::LearnMoreProposed;
}