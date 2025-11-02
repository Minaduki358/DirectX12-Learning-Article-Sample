#pragma once

#include"root_parameter_type.h"

struct RootParameterDescriptor
{
    RootParameterType type;
    UINT shaderRegister;              // b0, t0, u0 など
    UINT registerSpace = 0;           // space0, space1 など
    D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;

    // DescriptorTable用（SRVやCBVの配列）
    UINT descriptorRangeCount = 1;    // テーブル内の記述子数
    D3D12_DESCRIPTOR_RANGE_TYPE rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
};