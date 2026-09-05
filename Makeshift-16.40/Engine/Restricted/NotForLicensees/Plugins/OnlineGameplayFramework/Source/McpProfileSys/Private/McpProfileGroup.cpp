#include "pch.h"

void UMcpProfileGroup::SendRequestNowHook(UMcpProfileGroup* This, void* HttpRequest, EContextCredentials ContextCredentials)
{
	ContextCredentials = EContextCredentials::CXC_Public;

	return SendRequestNowOG(This, HttpRequest, ContextCredentials);
}

void UMcpProfileGroup::Init() {
	Memory::HookDetour(ImageBase + 0x144BE30, SendRequestNowHook, &SendRequestNowOG);
}
