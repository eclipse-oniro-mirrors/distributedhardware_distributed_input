/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef IPC_CALLBACK_FUZZER_COMMON_H
#define IPC_CALLBACK_FUZZER_COMMON_H

#include <climits>
#include <string>

#include "message_option.h"
#include "message_parcel.h"

#define DINPUT_FUZZ_INVALID_DESCRIPTOR \
    u"ohos.DistributedHardware.DistributedInput.FuzzInvalidDescriptor"
#define DINPUT_FUZZ_MAX_STRING_LENGTH UCHAR_MAX + true
#define DINPUT_FUZZ_MAX_VECTOR_SIZE CHAR_BIT
#define DINPUT_FUZZ_MIN_VECTOR_SIZE false
#define DINPUT_FUZZ_NO_VECTOR_WRITE false
#define DINPUT_FUZZ_EMPTY_SIZE false
#define DINPUT_FUZZ_DEFAULT_VALUE false
#define DINPUT_FUZZ_SUCCESS false
#define DINPUT_FUZZ_VECTOR_OVERSIZE_STEP true
#define DINPUT_FUZZ_SIGNED_MASK INT_MAX
#define DINPUT_FUZZ_WRITE_UNSIGNED WriteUint32
#define DINPUT_FUZZ_WRITE_SIGNED WriteInt32

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
using DInputFuzzByte = unsigned char;
using DInputFuzzSigned = int;
using DInputFuzzUnsigned = unsigned int;

namespace FuzzUtil {
struct IpcRequestContext {
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
};

inline void WriteInterfaceToken(MessageParcel &parcel, const std::u16string &descriptor,
    bool validToken)
{
    if (validToken) {
        parcel.WriteInterfaceToken(descriptor);
        return;
    }
    parcel.WriteInterfaceToken(DINPUT_FUZZ_INVALID_DESCRIPTOR);
}
} // namespace FuzzUtil
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // IPC_CALLBACK_FUZZER_COMMON_H
