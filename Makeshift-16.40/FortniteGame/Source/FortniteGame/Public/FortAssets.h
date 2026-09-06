#include "pch.h"

struct FFortAssets
{
	template<typename T>
	static T* GetAsset(const TSoftObjectPtr<T>& SoftObjectPtr);

	template<typename T>
	static void GetAssetArray(const TArray<TSoftObjectPtr<T>>& SoftObjectPtrs, TArray<T*>& OutAssets);
};

template<typename T>
T* FFortAssets::GetAsset(const TSoftObjectPtr<T>& SoftObjectPtr)
{
	UObject* Asset = SoftObjectPtr.Get();
	if (!Asset)
	{
		UObject* (*TryLoad)(const void*, bool) = decltype(TryLoad)(InSDKUtils::GetImageBase() + 0x16C527C);
		Asset = TryLoad(&SoftObjectPtr.ObjectID, true);
	}
	return Asset && Asset->IsA<T>() ? static_cast<T*>(Asset) : nullptr;
}

template<>
inline UFortItemDefToItemVariantDataMapping* FFortAssets::GetAsset(const TSoftObjectPtr<UFortItemDefToItemVariantDataMapping>& SoftObjectPtr)
{
	UFortItemDefToItemVariantDataMapping* (*Fn)(const TSoftObjectPtr<UFortItemDefToItemVariantDataMapping>*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x1F69060);
	return Fn(&SoftObjectPtr);
}

template<>
inline UDataTable* FFortAssets::GetAsset(const TSoftObjectPtr<UDataTable>& SoftObjectPtr)
{
	UDataTable* (*Fn)(const TSoftObjectPtr<UDataTable>*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x16C4C58);
	return Fn(&SoftObjectPtr);
}

template<>
inline void FFortAssets::GetAssetArray(const TArray<TSoftObjectPtr<UDataTable>>& SoftObjectPtrs, TArray<UDataTable*>& OutAssets)
{
	void (*Fn)(const TArray<TSoftObjectPtr<UDataTable>>*, TArray<UDataTable*>*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x454412C);
	Fn(&SoftObjectPtrs, &OutAssets);
}
