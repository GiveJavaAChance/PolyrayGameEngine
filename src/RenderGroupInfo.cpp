#include "RenderGroupInfo.h"
#include "RenderRequirements.h"
#include "SkinInstance.h"
#include "Transform3D.h"

RenderGroupInfo RenderGroupInfo::staticPBR() {
    return RenderGroupInfo::create<Transform3D, &Transform3D::global>(
        MaterialType::staticPBR(),
        RenderRequirements{
            AlphaMode::OPAQUE,
        });
}

RenderGroupInfo RenderGroupInfo::skinnedPBR() {
    return RenderGroupInfo::create<SkinInstance, &SkinInstance::data>(
        MaterialType::skinnedPBR(),
        RenderRequirements{
            AlphaMode::OPAQUE,
        });
}

RenderGroupInfo RenderGroupInfo::maskedStaticPBR() {
    return RenderGroupInfo::create<Transform3D, &Transform3D::global>(
        MaterialType::maskedStaticPBR(),
        RenderRequirements{
            AlphaMode::MASKED,
        });
}

RenderGroupInfo RenderGroupInfo::maskedSkinnedPBR() {
    return RenderGroupInfo::create<SkinInstance, &SkinInstance::data>(
        MaterialType::maskedSkinnedPBR(),
        RenderRequirements{
            AlphaMode::MASKED,
        });
}