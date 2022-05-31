#pragma once

#include "API.h"

namespace IUI::API
{
	template <typename Message> requires is_valid_message<Message>
	void DispatchMessage(const Message& a_message)
	{
		SKSE::GetMessagingInterface()->Dispatch(Message::type, const_cast<Message*>(&a_message),
												static_cast<std::uint32_t>(sizeof(Message)), nullptr);
	}
}
