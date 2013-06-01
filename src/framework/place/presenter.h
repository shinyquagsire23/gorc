#pragma once

#include "framework/event/eventbus.h"

namespace Gorc {
namespace Place {

class Presenter {
public:
	virtual ~Presenter();

	virtual void Start(Event::EventBus& eventBus) = 0;

	virtual bool CanStop();
	virtual void Stop();
};

}
}