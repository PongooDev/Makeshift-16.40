#include "pch.h"

struct FFortAssets
{
	template<typename T>
	static T* GetAsset(const TSoftObjectPtr<T>& SoftObjectPtr);
};

template<>
inline UFortItemDefToItemVariantDataMapping* FFortAssets::GetAsset(const TSoftObjectPtr<UFortItemDefToItemVariantDataMapping>& SoftObjectPtr)
{
	UFortItemDefToItemVariantDataMapping* (*Fn)(const TSoftObjectPtr<UFortItemDefToItemVariantDataMapping>*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x1F69060);
	return Fn(&SoftObjectPtr);
}
