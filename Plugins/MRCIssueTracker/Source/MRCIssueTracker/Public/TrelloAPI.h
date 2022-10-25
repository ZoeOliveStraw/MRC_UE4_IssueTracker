// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//Made by the Durham MRC, written by Zoe Straw

#pragma once

#include "http.h"
#include "Json.h"
#include "Engine.h"
#include "HttpModule.h"
#include "CoreMinimal.h"
#include "TrelloAPI.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MRCISSUETRACKER_API UTrelloAPI : public UObject
{
	GENERATED_BODY()
	
private:
	FString boardId;
	FString listId;
	FString cardId;

	bool validSoFar = true; //This can be used to check that the sequence of requests is still valid so far?

	
	
	void OnResponseReceived(FHttpRequestPtr request, FHttpResponsePtr response, bool success);
	void SetBoardIdFromName(FHttpRequestPtr request, FHttpResponsePtr response, bool success);
	void SetListIdFromName(FHttpRequestPtr request, FHttpResponsePtr response, bool success);
	void AttachScerenshotResponse(FHttpRequestPtr request, FHttpResponsePtr response, bool success);
	
public:
	UTrelloAPI();
	~UTrelloAPI();


	//We're going to use this to make sure that the requests we use throughout the process aren't failing
	//We can check this between requests to see if we should continue
	UPROPERTY(BlueprintReadOnly, Category="Trello API Functions")
	bool currentRequestValid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trello API Functions")
	FString boardName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trello Object Functions")
	FString listName;

	UFUNCTION(BlueprintCallable,Category="Trello API Functions")
	void SendTask(FString key, FString token, FString name, FString description); //Send a task to a trello board
	
	UFUNCTION(BlueprintCallable,Category="Trello API Functions")
	void GetBoards(FString key, FString token);
	
	UFUNCTION(BlueprintCallable,Category="Trello API Functions")
	void GetLists(FString key, FString token);

	UFUNCTION(BlueprintCallable,Category="Trello API Functions")
	void AttachScreenshot(FString key, FString token);
};

USTRUCT(BlueprintType)
struct FNameIDPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trello")
	FString name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trello")
	FString id;
};