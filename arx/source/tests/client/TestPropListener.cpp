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

#include <arx_debug.h>
#include <arx/ARXProperties.h>
#include <TestPropListener.h>

namespace tiarx {

TestPropListener::TestPropListener(event_t *event)
{
    mEvent = event;
}

void TestPropListener::onPropertyChanged(uint32_t property, int32_t value) {
    ARX_PRINT(ARX_ZONE_ALWAYS, "Property %d is now %d\n", property, value);
    if (property == PROP_ENGINE_STATE) {
        if (value == ENGINE_STATE_DEAD) {
            ARX_PRINT(ARX_ZONE_ERROR, "ARX died!\n");
            event_set(mEvent);
        } else if (value == ENGINE_STATE_STOP) {
            ARX_PRINT(ARX_ZONE_ALWAYS, "ARX stopped.\n");
            event_set(mEvent);
        }
    }
}

}
