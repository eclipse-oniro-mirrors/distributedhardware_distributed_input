/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dinput_struct_data.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
StringDeviceData::StringDeviceData() : dhname_(""), status(0) {}

bool StringDeviceData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(dhname_)) {
        DHLOGE("StringDeviceData-Marshalling write dhname_ failed");
        return false;
    }
    if (!parcel.WriteInt32(status)) {
        DHLOGE("StringDeviceData-Marshalling write status failed");
        return false;
    }
    return true;
}

sptr<StringDeviceData> StringDeviceData::Unmarshalling(Parcel &parcel)
{
    sptr<StringDeviceData> ptr = (std::make_unique<StringDeviceData>()).release();
    if (ptr == nullptr) {
        DHLOGE("StringDeviceData-Unmarshalling create ptr is null.");
        return nullptr;
    }

    if (!parcel.ReadString(ptr->dhname_)) {
        DHLOGE("StringDeviceData-Unmarshalling read dhname_ failed.");
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->status)) {
        DHLOGE("StringDeviceData-Unmarshalling read status failed.");
        return nullptr;
    }
    return ptr;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
