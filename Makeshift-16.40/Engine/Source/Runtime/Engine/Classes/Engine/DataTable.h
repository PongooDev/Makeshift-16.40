// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

	/** Get all of the rows in the table, regardless of name */
	template <class T>
	void UDataTable::GetAllRows(const TCHAR* ContextString, TArray<T*>& OutRowArray) const
	{
		if (RowStruct == nullptr)
		{
			UE_LOG(LogDataTable, Error, TEXT("UDataTable::GetAllRows : DataTable '%s' has no RowStruct specified (%s)."), *GetPathName(), ContextString);
		}
		else if (!RowStruct->IsChildOf(T::StaticStruct()))
		{
			UE_LOG(LogDataTable, Error, TEXT("UDataTable::GetAllRows : Incorrect type specified for DataTable '%s' (%s)."), *GetPathName(), ContextString);
		}
		else
		{
			OutRowArray.Reserve(OutRowArray.Num() + GetRowMap().Num());
			for (TMap<FName, uint8*>::TConstIterator RowMapIter(GetRowMap().CreateConstIterator()); RowMapIter; ++RowMapIter)
			{
				OutRowArray.Add(reinterpret_cast<T*>(RowMapIter.Value()));
			}
		}	
	}

	template <class T>
	void UDataTable::GetAllRows(const FString& ContextString, TArray<T*>& OutRowArray) const
	{
		GetAllRows<T>(*ContextString, OutRowArray);
	}
