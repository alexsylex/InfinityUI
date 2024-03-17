#pragma once

#include "API.h"

#undef DispatchMessage

namespace IUI::API
{
	template <typename MessageT> requires valid_message<MessageT>
	void DispatchMessage(const MessageT& a_message)
	{
		SKSE::GetMessagingInterface()->Dispatch(MessageT::type, const_cast<MessageT*>(&a_message),
												static_cast<std::uint32_t>(sizeof(MessageT)), nullptr);
	}
}
