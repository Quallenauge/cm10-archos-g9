/*
 *  Copyright (C) 2012 Texas Instruments, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <arx/ARAccelerator.h>
#include <sosal/event.h>

namespace tiarx {

/*!
 * Simple test property listener for ARX, which signals the given event
 * when ARX dies or stops.
 */
class TestPropListener : public ARXPropertyListener
{
public:
    TestPropListener(event_t *event);
    void onPropertyChanged(uint32_t property, int32_t value);

private:
    event_t *mEvent;
};

}
