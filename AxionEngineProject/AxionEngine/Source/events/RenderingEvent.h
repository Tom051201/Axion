#pragma once

#include "AxionEngine/Source/events/Event.h"

namespace Axion {

	class RenderingPreparedEvent : public Event {
	public:

		RenderingPreparedEvent() {}

		EVENT_CLASS_TYPE(RenderingPrepared);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	};

	class RenderingFinishedEvent : public Event {
	public:

		RenderingFinishedEvent() {}

		EVENT_CLASS_TYPE(RenderingFinished);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	
	};

}
