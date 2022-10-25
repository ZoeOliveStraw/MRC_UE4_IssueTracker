// Fill out your copyright notice in the Description page of Project Settings.


#include "TrelloAPI.h"

UTrelloAPI::UTrelloAPI()
{
	
}

UTrelloAPI::~UTrelloAPI()
{
	
}
//Sends a card through the Trello API 
void UTrelloAPI::SendTask(FString key, FString token, FString name, FString description)
{
	FString requestURL = TEXT("https://api.trello.com/1/cards?key="+ key +
		"&token=" + token +
		"&idList=" + listId +
		"&name=" + name +
		"&desc=" + description);

	requestURL = requestURL.Replace(TEXT(" "),TEXT("%20"));

	UE_LOG(LogTemp, Warning, TEXT("SEND TASK: List Id: %s"), *listId);
	
	FHttpRequestRef request = FHttpModule::Get().CreateRequest();
	request->OnProcessRequestComplete().BindUObject(this, &UTrelloAPI::OnResponseReceived);
	request->SetURL(requestURL);
	UE_LOG(LogTemp, Warning, TEXT("Request URL: %s"), *request->GetURL());
	request->SetHeader("Content-Type", TEXT("application/json"));
	request->SetVerb("POST");
	request->ProcessRequest();
}

void UTrelloAPI::GetBoards(FString key, FString token)
{
	currentRequestValid = false;
	
	FString requestURL = TEXT("https://api.trello.com/1/members/me/boards?key="+ key +"&token=" + token);
	
	FHttpRequestRef request = FHttpModule::Get().CreateRequest();
	request->OnProcessRequestComplete().BindUObject(this, &UTrelloAPI::SetBoardIdFromName);
	request->SetURL(requestURL);
	request->SetVerb("GET");
	request->ProcessRequest();
}

void UTrelloAPI::GetLists(FString key, FString token)
{
	currentRequestValid = false;
	
	UE_LOG(LogTemp,Warning,TEXT("Attempting to call GetLists with ID %s"), *boardId);
	FString requestURL = TEXT("https://api.trello.com/1/boards/" + boardId + "/lists?key="+ key +"&token=" + token);
	
	FHttpRequestRef request = FHttpModule::Get().CreateRequest();
	request->OnProcessRequestComplete().BindUObject(this, &UTrelloAPI::SetListIdFromName);
	request->SetURL(requestURL);
	request->SetVerb("GET");
	request->ProcessRequest();
}

//Send Task response
void UTrelloAPI::OnResponseReceived(FHttpRequestPtr request, FHttpResponsePtr response, bool success)
{
	TSharedPtr<FJsonObject> data;
	
	UE_LOG(LogTemp,Warning,TEXT("Response code: %i"), response->GetResponseCode());
	FString responseContentString = *response->GetContentAsString();
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(responseContentString);

	bool valid = FJsonSerializer::Deserialize(JsonReader, data);

	if(valid)
	{
		validSoFar = true;
		FString newCardId = data->GetStringField("id");
		cardId = newCardId;
		UE_LOG(LogTemp, Error, TEXT("New Card ID: %s"), *newCardId);
	}
	else
	{
		validSoFar = false;
		UE_LOG(LogTemp, Error, TEXT("Invalud input data for card ID"));
	}
}

void UTrelloAPI::SetBoardIdFromName(FHttpRequestPtr request, FHttpResponsePtr response, bool success)
{
	UE_LOG(LogTemp,Warning,TEXT("Response code: %i"), response->GetResponseCode());

	FString responseContentString = *response->GetContentAsString();

	TSharedRef<TJsonReader<TCHAR>> jsonReader = TJsonReaderFactory<TCHAR>::Create(responseContentString);
	TArray<TSharedPtr<FJsonValue>> boardArray;

	FJsonSerializer::Deserialize(jsonReader, boardArray);

	TArray<TSharedPtr<FJsonObject>> boardsAsJsonObjects;
	
	for(int i = 0; i < boardArray.Num(); i++)
	{
		TSharedPtr<FJsonObject> thisBoard = boardArray[i]->AsObject();
		FString nameToCheck = thisBoard->GetStringField("name");
		UE_LOG(LogTemp,Error,TEXT("Comparing %s to %s"),*nameToCheck, *boardName);
		if(boardName.Equals(thisBoard->GetStringField("name")))
		{
			boardId = thisBoard->GetStringField("id");
			UE_LOG(LogTemp,Error,TEXT("Board found with name %s id %s!"),*boardName, *boardId);
			currentRequestValid = true;
			return;
		}
	}
	UE_LOG(LogTemp,Error,TEXT("No board with name %s exists"), *boardName);
}

void UTrelloAPI::SetListIdFromName(FHttpRequestPtr request, FHttpResponsePtr response, bool success)
{
	UE_LOG(LogTemp,Warning,TEXT("Response code: %i"), response->GetResponseCode());

	FString responseContentString = *response->GetContentAsString();

	TSharedRef<TJsonReader<TCHAR>> jsonReader = TJsonReaderFactory<TCHAR>::Create(responseContentString);
	TArray<TSharedPtr<FJsonValue>> listArray;

	FJsonSerializer::Deserialize(jsonReader, listArray);

	TArray<TSharedPtr<FJsonObject>> boardsAsJsonObjects;

	for(int i = 0; i < listArray.Num(); i++)
	{
		TSharedPtr<FJsonObject> thisList = listArray[i]->AsObject();
		if(listName.Equals(thisList->GetStringField("name")))
		{
			FString thisListName = thisList->GetStringField("name");
			listId = thisList->GetStringField("id");
			UE_LOG(LogTemp,Error,TEXT("List found with name: %s, id: %s!"),*thisListName, *listId);
			currentRequestValid = true;
			return;
		}
	}
	UE_LOG(LogTemp,Error,TEXT("No list with name %s exists"), *listName);
}

//Take a screenshot to attach to the trello card that we're creating
void UTrelloAPI::AttachScreenshot(FString key, FString token)
{
	//Save the screenshot as ForTrello.png in screenshots directory
	const FString imageDirectory = FString::Printf(TEXT("%sForTrello.png"),*FPaths::ScreenShotDir());
	FScreenshotRequest::RequestScreenshot(imageDirectory,true,false);

	//Prepare screenshot to upload with the request
	TArray<uint8> fileBinary;
	FFileHelper::LoadFileToArray(fileBinary, *imageDirectory);

	//Create a Json we can make
	FString requestJsonData;
	const TSharedRef<TJsonWriter<TCHAR>> jsonWriter = TJsonWriterFactory<TCHAR>::Create(&requestJsonData);

	const FString& valueName = TEXT("name");
	const FString valueScreenshot = TEXT("Screenshot.");

	const FString& idName = TEXT("id");
	const FString idValue = FString::Printf(TEXT("%s"),*cardId);

	
	
	//Start loading information into that Json
	jsonWriter->WriteObjectStart();
	jsonWriter->WriteValue(idName,idValue);
	jsonWriter->WriteValue(valueName,valueScreenshot);
	jsonWriter->WriteValue("file",FBase64::Encode(fileBinary));
	jsonWriter->WriteObjectEnd();
	jsonWriter->Close();

	UE_LOG(LogTemp,Warning, TEXT("Sending request as: %s"), *requestJsonData)

	FString arrayRequestJsonData = FString::Printf(TEXT("[%s]"), *requestJsonData);
	
	FString requestURL = TEXT("https://api.trello.com/1/cards/" + cardId + "/attachments?key="+ key +"&token=" + token);
	
	FHttpRequestRef request = FHttpModule::Get().CreateRequest();

	request->OnProcessRequestComplete().BindUObject(this, &UTrelloAPI::OnResponseReceived);
	request->SetURL(requestURL);
	UE_LOG(LogTemp, Warning, TEXT("Request URL: %s"), *request->GetURL());
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	request->SetVerb("POST");
	request->SetContentAsString(requestJsonData);
	//request->SetContent(fileBinary);
	request->ProcessRequest();
	
}

void UTrelloAPI::AttachScerenshotResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool success)
{
	UE_LOG(LogTemp,Warning,TEXT("Response code: %i"), response->GetResponseCode());
}
