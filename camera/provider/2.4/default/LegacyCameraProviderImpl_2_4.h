/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_CAMERA_PROVIDER_V2_4_LEGACYCAMERAPROVIDER_H
#define ANDROID_HARDWARE_CAMERA_PROVIDER_V2_4_LEGACYCAMERAPROVIDER_H

#include <set>
#include <android/hardware/camera/provider/2.4/ICameraProvider.h>
#include "hardware/camera_common.h"
#include "utils/Mutex.h"
#include "utils/SortedVector.h"

#include "CameraModule.h"
#include "VendorTagDescriptor.h"

namespace android {
namespace hardware {
namespace camera {
namespace provider {
namespace V2_4 {
namespace implementation {

using ::android::hardware::camera::common::V1_0::CameraDeviceStatus;
using ::android::hardware::camera::common::V1_0::Status;
using ::android::hardware::camera::common::V1_0::TorchModeStatus;
using ::android::hardware::camera::common::V1_0::VendorTag;
using ::android::hardware::camera::common::V1_0::VendorTagSection;
using ::android::hardware::camera::common::V1_0::helper::CameraModule;
using ::android::hardware::camera::common::V1_0::helper::VendorTagDescriptor;
using ::android::hardware::camera::provider::V2_4::ICameraProvider;
using ::android::hardware::camera::provider::V2_4::ICameraProviderCallback;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::sp;
using ::android::Mutex;

/**
 * The implementation of legacy wrapper CameraProvider 2.4, separated
 * from the HIDL interface layer to allow for implementation reuse by later
 * provider versions.
 *
 * This implementation supports cameras implemented via the legacy libhardware
 * camera HAL definitions.
 */
struct LegacyCameraProviderImpl_2_4 : public camera_module_callbacks_t {
    LegacyCameraProviderImpl_2_4();
    ~LegacyCameraProviderImpl_2_4();

    // Caller must use this method to check if CameraProvider ctor failed
    bool isInitFailed() { return mInitFailed; }

    // Methods from ::android::hardware::camera::provider::V2_4::ICameraProvider follow.
    Return<Status> setCallback(const sp<ICameraProviderCallback>& callback);
    Return<void> getVendorTags(ICameraProvider::getVendorTags_cb _hidl_cb);
    Return<void> getCameraIdList(ICameraProvider::getCameraIdList_cb _hidl_cb);
    Return<void> isSetTorchModeSupported(ICameraProvider::isSetTorchModeSupported_cb _hidl_cb);
    Return<void> getCameraDeviceInterface_V1_x(
            const hidl_string& cameraDeviceName,
            ICameraProvider::getCameraDeviceInterface_V1_x_cb _hidl_cb);
    Return<void> getCameraDeviceInterface_V3_x(
            const hidl_string& cameraDeviceName,
            ICameraProvider::getCameraDeviceInterface_V3_x_cb _hidl_cb);

protected:
    Mutex mCbLock;
    sp<ICameraProviderCallback> mCallbacks = nullptr;

    sp<CameraModule> mModule;

    int mNumberOfLegacyCameras;
    std::map<std::string, camera_device_status_t> mCameraStatusMap; // camera id -> status
    std::map<std::string, bool> mOpenLegacySupported; // camera id -> open_legacy HAL1.0 supported
    std::set<int> mLegacyCameras; // non-external cameras
    SortedVector<std::string> mCameraIds; // the "0"/"1" legacy camera Ids
    // (cameraId string, hidl device name) pairs
    SortedVector<std::pair<std::string, std::string>> mCameraDeviceNames;

    int mPreferredHal3MinorVersion;

    // Must be queried before using any APIs.
    // APIs will only work when this returns true
    bool mInitFailed;
    bool initialize();

    hidl_vec<VendorTagSection> mVendorTagSections;
    bool setUpVendorTags();
    int checkCameraVersion(int id, camera_info info);

    // create HIDL device name from camera ID and legacy device version
    std::string getHidlDeviceName(std::string cameraId, int deviceVersion);

    // extract legacy camera ID/device version from a HIDL device name
    static std::string getLegacyCameraId(const hidl_string& deviceName);

    // convert conventional HAL status to HIDL Status
    static Status getHidlStatus(int);

    // static callback forwarding methods
    static void sCameraDeviceStatusChange(
        const struct camera_module_callbacks* callbacks,
        int camera_id,
        int new_status);
    static void sTorchModeStatusChange(
        const struct camera_module_callbacks* callbacks,
        const char* camera_id,
        int new_status);

    void addDeviceNames(int camera_id, CameraDeviceStatus status = CameraDeviceStatus::PRESENT,
                        bool cam_new = false);
    void removeDeviceNames(int camera_id);

};

}  // namespace implementation
}  // namespace V2_4
}  // namespace provider
}  // namespace camera
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_CAMERA_PROVIDER_V2_4_LEGACYCAMERAPROVIDER_H
