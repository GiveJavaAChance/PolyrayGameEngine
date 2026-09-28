#include <GltfLoader.h>

#include <cstring>

#include <fastgltf/core.h>
#include <fastgltf/tools.h>
#include <fastgltf/types.h>

#include <Camera3D.h>
#include <MaterialType.h>
#include <Renderer.h>
#include <ResourceManager.h>
#include <SkinSystem.h>
#include <Transform3D.h>
#include <World.h>
#include <animation/AnimationSystem.h>
#include <gpu_types/GpuPBRMaterial.h>
#include <light/3d/Light3DSystem.h>
#include <scene/3d/Scene3D.h>
#include <shader/ShaderManager.h>
#include <structure/DynamicArray.h>

fastgltf::Asset loadAsset(const ResourcePath& res) {
    std::filesystem::path path = res.getAbsolutePath();
    if (!std::filesystem::exists(path)) {
        std::cerr << "Failed to find file: " << path << std::endl;
        std::exit(1);
    }
    static constexpr fastgltf::Extensions supportedExtensions =
        fastgltf::Extensions::KHR_mesh_quantization |
        fastgltf::Extensions::KHR_texture_transform |
        fastgltf::Extensions::KHR_materials_variants |
        fastgltf::Extensions::KHR_lights_punctual;

    fastgltf::Parser parser(supportedExtensions);

    constexpr fastgltf::Options gltfOptions =
        fastgltf::Options::DontRequireValidAssetMember |
        fastgltf::Options::AllowDouble |
        fastgltf::Options::LoadExternalBuffers |
        fastgltf::Options::GenerateMeshIndices;

    fastgltf::Expected<fastgltf::MappedGltfFile> gltfFile = fastgltf::MappedGltfFile::FromPath(path);
    if (!bool(gltfFile)) {
        std::cerr << "Failed to open glTF file: " << fastgltf::getErrorMessage(gltfFile.error()) << '\n';
        std::exit(1);
    }
    fastgltf::Expected<fastgltf::Asset> asset = parser.loadGltf(gltfFile.get(), path.parent_path(), gltfOptions);
    if (asset.error() != fastgltf::Error::None) {
        std::cerr << "Failed to load glTF: " << fastgltf::getErrorMessage(asset.error()) << '\n';
        std::exit(1);
    }
    return std::move(asset.get());
}

uint32_t loadNode(const fastgltf::Asset& asset, const fastgltf::Node& node, ECS& ecs) {
    Entity e = ecs.createEntity();
    e.addComponent(std::visit(
        fastgltf::visitor{
            [&](const mat4& matrix) {
                Transform3D tx;
                decomposeTransformMatrix(matrix, tx.scale, tx.rotation, tx.position);
                return tx;
            },
            [&](const fastgltf::TRS& trs) {
                return Transform3D{trs.translation, trs.rotation, trs.scale};
            }},
        node.transform));
    return e.entityID;
}

void loadScene(const fastgltf::Asset& asset, std::size_t nodeIndex, const DynamicArray<uint32_t>& nodes, World* world, uint32_t fromNode) {
    const fastgltf::Node& node = asset.nodes[nodeIndex];

    Scene3D* sceneSystem = world->getSystem<Scene3D>();

    uint32_t nodeID = sceneSystem->addNode(fromNode, Entity{nodes[nodeIndex], &world->ecs}, std::string(node.name));

    for (std::size_t i : node.children) {
        loadScene(asset, i, nodes, world, nodeID);
    }
}

uint32_t loadScene(const fastgltf::Asset& asset, const fastgltf::Scene& scene, const DynamicArray<uint32_t>& nodes, World* world, uint32_t fromNode) {
    Entity e = world->ecs.createEntity();
    e.addComponent(Transform3D{});

    Scene3D* sceneSystem = world->getSystem<Scene3D>();

    uint32_t sceneRoot = sceneSystem->addNode(fromNode, e, std::string(scene.name));

    for (std::size_t i : scene.nodeIndices) {
        loadScene(asset, i, nodes, world, sceneRoot);
    }

    return sceneRoot;
}

Camera3D loadCamera(fastgltf::Camera& camera) {
    Camera3D cam{};
    std::visit(
        fastgltf::visitor{
            [&](fastgltf::Camera::Perspective& perspective) {
                cam = Camera3D::perspective(perspective.znear, 57.2957795131f * perspective.yfov);
            },
            [&](fastgltf::Camera::Orthographic& orthographic) {
                cam = Camera3D::orthographic(2.0f * orthographic.xmag, orthographic.znear, orthographic.zfar);
            },
        },
        camera.camera);
    return cam;
}

GltfLight loadLight(fastgltf::Light& light) {
    GltfLight l{};
    switch (light.type) {
        case fastgltf::LightType::Directional: {
            l.type = GltfLight::Type::DIRECTIONAL;
            l.directional = DirectionalLight3D(light.color, light.intensity);
            break;
        }
        case fastgltf::LightType::Spot: {
            l.type = GltfLight::Type::SPOT;
            l.spot = SpotLight3D(light.color, light.intensity, 0.1f, light.outerConeAngle.value() * 2.0f);
            break;
        }
        case fastgltf::LightType::Point: {
            l.type = GltfLight::Type::POINT;
            l.point = PointLight3D(light.color, light.intensity, 0.1f);
            break;
        }
    }
    return l;
}

GLTexture loadImage(const ResourcePath& parentPath, const fastgltf::Asset& asset, fastgltf::Image& image) {
    GLTexture texture;
    std::visit(
        fastgltf::visitor{
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filePath) {
                assert(filePath.fileByteOffset == 0);

                texture = ResourceManager::getResourceAsTexture(parentPath / std::string(filePath.uri.string()), -1, GL_RGBA8, false);
            },
            [&](fastgltf::sources::Array& vector) {
                int width, height, nrChannels;

                stbi_set_flip_vertically_on_load(false);
                stbi_uc* data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data()), static_cast<int>(vector.bytes.size()), &width, &height, &nrChannels, 4);
                texture = GLTexture::createTexture2D(width, height, GL_RGBA8, -1);
                texture.set2DTextureData(data, width, height, 0, 0, GL_UNSIGNED_BYTE, GL_RGBA);
                stbi_image_free(data);
            },
            [&](fastgltf::sources::BufferView& view) {
                const fastgltf::BufferView& bufferView = asset.bufferViews[view.bufferViewIndex];
                const fastgltf::Buffer& buffer = asset.buffers[bufferView.bufferIndex];
                std::visit(
                    fastgltf::visitor{
                        [](auto& arg) {},
                        [&](fastgltf::sources::Array& vector) {
                            int width, height, nrChannels;

                            stbi_set_flip_vertically_on_load(false);
                            stbi_uc* data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data() + bufferView.byteOffset), static_cast<int>(bufferView.byteLength), &width, &height, &nrChannels, 4);
                            texture = GLTexture::createTexture2D(width, height, GL_RGBA8, -1);
                            texture.set2DTextureData(data, width, height, 0, 0, GL_UNSIGNED_BYTE, GL_RGBA);
                            stbi_image_free(data);
                        }},
                    buffer.data);
            },
        },
        image.data);
    texture.setWrapMode(GL_REPEAT);
    texture.setAnisotrophy(GLTexture::getMaxAnisotropy());
    texture.setInterpolation(true);
    texture.generateMipmap();
    return texture;
}

GpuUvTransform getUvTransform(const fastgltf::Optional<fastgltf::TextureInfo>& texture) {
    GpuUvTransform result{};
    if (texture.has_value() && texture->transform) {
        const fastgltf::TextureTransform& t = *texture->transform;

        result.offset = t.uvOffset;
        result.scale = t.uvScale;
        result.rotation = static_cast<float>(t.rotation);
    }
    return result;
}

GpuUvTransform getUvTransform(const fastgltf::Optional<fastgltf::NormalTextureInfo>& texture) {
    GpuUvTransform result{};
    if (texture.has_value() && texture->transform) {
        const fastgltf::TextureTransform& t = *texture->transform;

        result.offset = t.uvOffset;
        result.scale = t.uvScale;
        result.rotation = static_cast<float>(t.rotation);
    }
    return result;
}

GpuPBRMaterial loadMaterial(const DynamicArray<GLTexture>& textures, const fastgltf::Material& material) {
    GpuPBRMaterial mat;
    mat.baseColor = material.pbrData.baseColorFactor;

    if (material.pbrData.baseColorTexture.has_value()) {
        mat.baseColorTexture = textures[material.pbrData.baseColorTexture->textureIndex].getBindlessHandle();
    } else {
        mat.baseColorTexture = 0ull;
    }
    if (material.normalTexture.has_value()) {
        mat.normalMapTexture = textures[material.normalTexture->textureIndex].getBindlessHandle();
    } else {
        mat.normalMapTexture = 0ull;
    }
    mat.baseMetallicRougness = prvl::vec2(material.pbrData.metallicFactor, material.pbrData.roughnessFactor);
    if (material.pbrData.metallicRoughnessTexture.has_value()) {
        mat.metallicRoughnessTexture = textures[material.pbrData.metallicRoughnessTexture->textureIndex].getBindlessHandle();
    } else {
        mat.metallicRoughnessTexture = 0ull;
    }

    mat.alphaCutoff = material.alphaCutoff;
    mat.doubleSided = material.doubleSided;

    mat.baseColorUvTransform = getUvTransform(material.pbrData.baseColorTexture);
    mat.normalUvTransform = getUvTransform(material.normalTexture);
    mat.metallicRoughnessUvTransform = getUvTransform(material.pbrData.metallicRoughnessTexture);

    mat.F0 = prvl::vec3(0.05f);

    return mat;
}

std::string getAttributeName(fastgltf::Asset& asset, const fastgltf::Primitive& primitive, const std::string& attribute) {
    if (attribute == "TEXCOORD") {
        size_t uvSet = 0;
        if (primitive.materialIndex.has_value()) {
            fastgltf::Material& material = asset.materials[primitive.materialIndex.value()];
            fastgltf::Optional<fastgltf::TextureInfo>& baseColorTexture = material.pbrData.baseColorTexture;
            if (baseColorTexture.has_value()) {
                if (baseColorTexture->transform && baseColorTexture->transform->texCoordIndex.has_value()) {
                    uvSet = baseColorTexture->transform->texCoordIndex.value();
                } else {
                    uvSet = material.pbrData.baseColorTexture->texCoordIndex;
                }
            }
        }
        return "TEXCOORD_" + std::to_string(uvSet);
    }
    if (attribute == "COLOR") {
        return "COLOR_0";
    }
    if (attribute == "JOINTS") {
        return "JOINTS_0";
    }
    if (attribute == "WEIGHTS") {
        return "WEIGHTS_0";
    }
    return attribute;
}

void loadMeshes(const DynamicArray<RenderGroupInfo>& renderGroups, const DynamicArray<GpuPBRMaterial>& materials, fastgltf::Asset& asset, const fastgltf::Mesh& mesh, DynamicArray<DynamicArray<uint32_t>>& renderObjects, World* world) {
    Renderer* renderer = world->getSystem<Renderer>();

    renderObjects.emplace();
    DynamicArray<uint32_t>& objects = renderObjects[renderObjects.size() - 1u];
    for (std::size_t primitiveIndex = 0; primitiveIndex < mesh.primitives.size(); ++primitiveIndex) {
        const fastgltf::Primitive& primitive = mesh.primitives[primitiveIndex];

        const fastgltf::Attribute* positionAttribute = primitive.findAttribute("POSITION");
        if (positionAttribute == primitive.attributes.end()) {
            continue;
        }
        uint32_t vertexCount = static_cast<uint32_t>(asset.accessors[positionAttribute->accessorIndex].count);

        VertexLayoutInfo* vertexLayout = nullptr;
        uint32_t matchIdx = 0u;
        uint32_t maxMatch = 0u;

        for (uint32_t i = 0u; i < renderGroups.size(); i++) {
            VertexLayoutInfo& layout = renderGroups[i].materialType.vertexLayout;
            uint32_t matched = 0u;
            for (uint32_t j = 0u; j < layout.attributes.size(); j++) {
                VertexAttribInfo& attribute = layout.attributes[j];
                if (attribute.instanced) {
                    continue;
                }
                const fastgltf::Attribute* attrib = primitive.findAttribute(getAttributeName(asset, primitive, attribute.name));
                if (attrib == primitive.attributes.end() && !attribute.optional) {
                    goto next;
                }
                matched++;
            }
            if (matched > maxMatch) {
                vertexLayout = &layout;
                matchIdx = i;
                maxMatch = matched;
            }
        next:;
        }

        if (!vertexLayout) {
            std::cerr << "No material type found!" << std::endl;
            continue;
        }

        uint32_t vertexSize = 0u;

        for (uint32_t i = 0u; i < vertexLayout->attributes.size(); i++) {
            VertexAttribInfo& attribute = vertexLayout->attributes[i];
            if (attribute.instanced) {
                continue;
            }
            vertexSize += attribute.columns * attribute.byteSize;
        }

        uint32_t vertexBufferSize = vertexCount * vertexSize;
        uint8_t* vertexData = alloc<uint8_t>(vertexBufferSize);

        uint32_t baseOffset = 0u;

        for (uint32_t i = 0u; i < vertexLayout->attributes.size(); i++) {
            VertexAttribInfo& attribute = vertexLayout->attributes[i];
            if (attribute.instanced) {
                continue;
            }
            const fastgltf::Attribute* attrib = primitive.findAttribute(getAttributeName(asset, primitive, attribute.name));
            if (attrib == primitive.attributes.end()) {
                baseOffset += attribute.columns * attribute.byteSize;
                continue;
            }
            const fastgltf::Accessor& accessor = asset.accessors[attrib->accessorIndex];
            if (attribute.name == "COLOR") {
                if (accessor.type == fastgltf::AccessorType::Vec3) {
                    fastgltf::iterateAccessorWithIndex<vec3>(asset, accessor,
                        [&](vec3 value, std::size_t index) {
                            vec4 color = prvl::vec4(value, 1.0f);
                            std::memcpy(vertexData + baseOffset + vertexSize * index, &color, attribute.byteSize);
                        });
                } else {
                    fastgltf::iterateAccessorWithIndex<vec4>(asset, accessor,
                        [&](vec4 value, std::size_t index) {
                            std::memcpy(vertexData + baseOffset + vertexSize * index, &value, attribute.byteSize);
                        });
                }
                baseOffset += attribute.columns * attribute.byteSize;
                continue;
            }
            if (attribute.name == "TANGENT") {
                fastgltf::iterateAccessorWithIndex<vec4>(asset, accessor,
                    [&](vec4 value, std::size_t index) {
                        std::memcpy(vertexData + baseOffset + vertexSize * index, &value, attribute.byteSize);
                    });
                baseOffset += attribute.columns * attribute.byteSize;
                continue;
            }
            if (attribute.baseType == GL_FLOAT) {
                switch (attribute.components) {
                    case 1: {
                        fastgltf::iterateAccessorWithIndex<float>(asset, accessor,
                            [&](float value, std::size_t index) {
                                std::memcpy(vertexData + baseOffset + vertexSize * index, &value, attribute.byteSize);
                            });
                        break;
                    }
                    case 2: {
                        fastgltf::iterateAccessorWithIndex<vec2>(asset, accessor,
                            [&](vec2 value, std::size_t index) {
                                std::memcpy(vertexData + baseOffset + vertexSize * index, &value, attribute.byteSize);
                            });
                        break;
                    }
                    case 3: {
                        fastgltf::iterateAccessorWithIndex<vec3>(asset, accessor,
                            [&](vec3 value, std::size_t index) {
                                std::memcpy(vertexData + baseOffset + vertexSize * index, &value, attribute.byteSize);
                            });
                        break;
                    }
                    case 4: {
                        fastgltf::iterateAccessorWithIndex<vec4>(asset, accessor,
                            [&](vec4 value, std::size_t index) {
                                std::memcpy(vertexData + baseOffset + vertexSize * index, &value, attribute.byteSize);
                            });
                        break;
                    }
                }
            } else if (attribute.baseType == GL_UNSIGNED_BYTE || attribute.baseType == GL_UNSIGNED_SHORT || attribute.baseType == GL_UNSIGNED_INT) {
                switch (attribute.components) {
                    case 1: {
                        fastgltf::iterateAccessorWithIndex<uint32_t>(asset, accessor,
                            [&](uint32_t value, std::size_t index) {
                                std::memcpy(vertexData + baseOffset + vertexSize * index, &value, attribute.byteSize);
                            });
                        break;
                    }
                    case 2: {
                        fastgltf::iterateAccessorWithIndex<uvec2>(asset, accessor,
                            [&](uvec2 value, std::size_t index) {
                                std::memcpy(vertexData + baseOffset + vertexSize * index, &value, attribute.byteSize);
                            });
                        break;
                    }
                    case 3: {
                        fastgltf::iterateAccessorWithIndex<uvec3>(asset, accessor,
                            [&](uvec3 value, std::size_t index) {
                                std::memcpy(vertexData + baseOffset + vertexSize * index, &value, attribute.byteSize);
                            });
                        break;
                    }
                    case 4: {
                        fastgltf::iterateAccessorWithIndex<uvec4>(asset, accessor,
                            [&](uvec4 value, std::size_t index) {
                                std::memcpy(vertexData + baseOffset + vertexSize * index, &value, attribute.byteSize);
                            });
                        break;
                    }
                }
            }
            baseOffset += attribute.columns * attribute.byteSize;
        }

        const fastgltf::Accessor& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];
        uint32_t* indices = alloc<uint32_t>(indexAccessor.count);
        fastgltf::copyFromAccessor<uint32_t>(asset, indexAccessor, indices);

        uint32_t materialIndex = 0u;
        if (!primitive.mappings.empty() && primitive.mappings[0u].has_value()) {
            materialIndex = primitive.mappings[0u].value();
        } else {
            materialIndex = primitive.materialIndex.value();
        }

        GpuPBRMaterial& mat = materials[materialIndex];
        RenderState renderState{};
        renderState.doubleSided = static_cast<bool>(mat.doubleSided);

        uint32_t renderGroup = renderer->getOrCreateGroup(renderGroups[matchIdx]);
        uint32_t materialID = renderer->addMaterialInstance(renderGroup, mat, renderState);
        uint32_t object = renderer->createObject(renderGroup, materialID);
        objects.add(object);
        RenderObject& obj = renderer->getObject(object);
        obj.vbo.uploadData(vertexData, vertexBufferSize);
        obj.ebo.uploadData(indices, indexAccessor.count);
        obj.vertexCount = vertexCount;
        obj.indexCount = indexAccessor.count;
        free(vertexData);
        free(indices);
    }
}

uint32_t loadAnimation(const fastgltf::Animation& animation, const fastgltf::Asset& asset, uint32_t rootNode, const DynamicArray<uint32_t>& nodes, uint32_t groupID, World* world) {
    Scene3D* scene = world->getSystem<Scene3D>();
    AnimationSystem* animationSystem = world->getSystem<AnimationSystem>();
    uint32_t animationID = animationSystem->createAnimation(groupID, std::string(animation.name));
    Animation& anim = animationSystem->getAnimation(animationID);

    for (const fastgltf::AnimationSampler& sampler : animation.samplers) {
        const fastgltf::Accessor& input = asset.accessors[sampler.inputAccessor];

        if (input.max.has_value()) {
            anim.duration = max(anim.duration, static_cast<float>(input.max.value().get<double>(0u)));
        }
    }

    for (const fastgltf::AnimationChannel& channel : animation.channels) {
        if (!channel.nodeIndex.has_value()) {
            continue;
        }
        const fastgltf::AnimationSampler& sampler = animation.samplers[channel.samplerIndex];

        AnimationChannel& chan = anim.channels.emplace();

        uint32_t nodeID = scene->getNode(nodes[static_cast<uint32_t>(channel.nodeIndex.value())]);
        chan.binding.targetPath = scene->getNodePath(nodeID, rootNode);

        switch (sampler.interpolation) {
            case fastgltf::AnimationInterpolation::Step: {
                chan.interpolation = Interpolation::STEP;
                break;
            }
            case fastgltf::AnimationInterpolation::Linear: {
                chan.interpolation = Interpolation::LINEAR;
                break;
            }
            case fastgltf::AnimationInterpolation::CubicSpline: {
                chan.interpolation = Interpolation::CUBIC_SPLINE;
                break;
            }
            default: {
                break;
            }
        }

        struct CubicSplineVec3 {
            vec3 inTangent;
            vec3 value;
            vec3 outTangent;
        };
        struct CubicSplineQuat {
            quat inTangent;
            quat value;
            quat outTangent;
        };

        const fastgltf::Accessor& input = asset.accessors[sampler.inputAccessor];
        const fastgltf::Accessor& output = asset.accessors[sampler.outputAccessor];
        fastgltf::iterateAccessor<float>(asset, input, [&](float time) {
            chan.keyframeTimes.add(time);
        });
        switch (channel.path) {
            case fastgltf::AnimationPath::Translation: {
                chan.binding.setup<Transform3D, vec3, &Transform3D::position>();
                chan.binding.setDirtyFlagMember<Transform3D, bool, &Transform3D::dirtyTRS>(1u);

                if (chan.interpolation == Interpolation::CUBIC_SPLINE) {
                    chan.setInterpolator<CubicSplineVec3, vec3>(
                        [](const CubicSplineVec3* a, const CubicSplineVec3* b, vec3* c, float t, float dt) {
                            float t2 = t * t;
                            float t3 = t2 * t;

                            float m0 = 2.0f * t3 - 3.0f * t2 + 1.0f;
                            float m1 = dt * (t3 - 2.0f * t2 + t);
                            float m2 = -2.0f * t3 + 3.0f * t2;
                            float m3 = dt * (t3 - t2);

                            *c = m0 * a->value + m1 * a->outTangent + m2 * b->value + m3 * b->inTangent;
                        },
                        [](const CubicSplineVec3* in, vec3* out) {
                            *out = in->value;
                        });
                } else {
                    chan.setInterpolator<vec3>(
                        [](const vec3* a, const vec3* b, vec3* c, float t, float dt) {
                            *c = mix(*a, *b, t);
                        });
                }
                fastgltf::iterateAccessor<vec3>(asset, output, [&](vec3 value) {
                    chan.keyframes.addAll(reinterpret_cast<const uint8_t*>(&value), sizeof(vec3));
                });
                break;
            }
            case fastgltf::AnimationPath::Rotation: {
                chan.binding.setup<Transform3D, quat, &Transform3D::rotation>();
                chan.binding.setDirtyFlagMember<Transform3D, bool, &Transform3D::dirtyTRS>(1u);
                if (chan.interpolation == Interpolation::CUBIC_SPLINE) {
                    chan.setInterpolator<CubicSplineQuat, quat>(
                        [](const CubicSplineQuat* a, const CubicSplineQuat* b, quat* c, float t, float dt) {
                            float t2 = t * t;
                            float t3 = t2 * t;

                            float m0 = 2.0f * t3 - 3.0f * t2 + 1.0f;
                            float m1 = dt * (t3 - 2.0f * t2 + t);
                            float m2 = -2.0f * t3 + 3.0f * t2;
                            float m3 = dt * (t3 - t2);

                            *c = normalize(m0 * a->value + m1 * dt * a->outTangent + m2 * b->value + m3 * dt * b->inTangent);
                        },
                        [](const CubicSplineQuat* in, quat* out) {
                            *out = in->value;
                        });
                } else {
                    chan.setInterpolator<quat>(
                        [](const quat* a, const quat* b, quat* c, float t, float dt) {
                            *c = slerp(*a, *b, t);
                        });
                }
                fastgltf::iterateAccessor<quat>(asset, output, [&](quat value) {
                    chan.keyframes.addAll(reinterpret_cast<const uint8_t*>(&value), sizeof(quat));
                });
                break;
            }
            case fastgltf::AnimationPath::Scale: {
                chan.binding.setup<Transform3D, vec3, &Transform3D::scale>();
                chan.binding.setDirtyFlagMember<Transform3D, bool, &Transform3D::dirtyTRS>(1u);
                if (chan.interpolation == Interpolation::CUBIC_SPLINE) {
                    chan.setInterpolator<CubicSplineVec3, vec3>(
                        [](const CubicSplineVec3* a, const CubicSplineVec3* b, vec3* c, float t, float dt) {
                            float t2 = t * t;
                            float t3 = t2 * t;

                            float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
                            float h10 = t3 - 2.0f * t2 + t;
                            float h01 = -2.0f * t3 + 3.0f * t2;
                            float h11 = t3 - t2;

                            *c = h00 * a->value + (h10 * dt) * a->outTangent + h01 * b->value + (h11 * dt) * b->inTangent;
                        },
                        [](const CubicSplineVec3* in, vec3* out) {
                            *out = in->value;
                        });
                } else {
                    chan.setInterpolator<vec3>(
                        [](const vec3* a, const vec3* b, vec3* c, float t, float dt) {
                            *c = mix(*a, *b, t);
                        });
                }
                fastgltf::iterateAccessor<vec3>(asset, output, [&](vec3 value) {
                    chan.keyframes.addAll(reinterpret_cast<const uint8_t*>(&value), sizeof(vec3));
                });
                break;
            }
            case fastgltf::AnimationPath::Weights: {
                chan.setInterpolator<float>(
                    [](const float* a, const float* b, float* c, float t, float dt) {
                        *c = mix(*a, *b, t);
                    });
                fastgltf::iterateAccessor<float>(asset, output, [&](float value) {
                    chan.keyframes.addAll(reinterpret_cast<const uint8_t*>(&value), sizeof(float));
                });
                break;
            }
        }
    }
    return animationID;
}

uint32_t loadSkin(const fastgltf::Asset& asset, const fastgltf::Skin& skin, const DynamicArray<uint32_t>& nodes, World* world) {
    SkinSystem* skinSystem = world->getSystem<SkinSystem>();
    uint32_t skinID = skinSystem->createSkin(skin.joints.size());
    Skin& s = skinSystem->getSkin(skinID);
    s.transformIDs.reserve(skin.joints.size());
    for (uint32_t i = 0u; i < skin.joints.size(); i++) {
        world->ecs.getComponentID<Transform3D>(nodes[skin.joints[i]], s.transformIDs[i]);
    }
    if (skin.inverseBindMatrices.has_value()) {
        const fastgltf::Accessor& accessor = asset.accessors[skin.inverseBindMatrices.value()];
        s.inverseBindMatrices.reserve(accessor.count);
        fastgltf::iterateAccessorWithIndex<mat4>(asset, accessor,
            [&](mat4 matrix, std::size_t index) {
                s.inverseBindMatrices[index] = matrix;
            });
    }
    return skinID;
}

uint32_t GltfLoader::load(const ResourcePath& res, World* world, uint32_t fromNode, const DynamicArray<RenderGroupInfo>& renderGroups) {
    fastgltf::Asset asset = loadAsset(res);
    std::string assetName = res.getName();
    assetName = assetName.substr(0u, assetName.find_last_of('.'));

    ResourcePath parent = res.getParent();

    ECS& ecs = world->ecs;
    DynamicArray<uint32_t> nodes;
    for (fastgltf::Node& node : asset.nodes) {
        nodes.add(loadNode(asset, node, ecs));
    }

    Scene3D* sceneSystem = world->getSystem<Scene3D>();

    Entity root = ecs.createEntity();
    root.addComponent(Transform3D{});
    uint32_t rootNode = sceneSystem->addNode(fromNode, root, assetName);

    for (fastgltf::Scene& scene : asset.scenes) {
        loadScene(asset, scene, nodes, world, rootNode);
    }

    DynamicArray<Camera3D> cameras;
    for (fastgltf::Camera& camera : asset.cameras) {
        cameras.add(loadCamera(camera));
    }

    DynamicArray<GltfLight> lights;
    for (fastgltf::Light& light : asset.lights) {
        lights.add(loadLight(light));
    }

    DynamicArray<GLTexture> textures;
    for (fastgltf::Image& image : asset.images) {
        textures.add(loadImage(parent, asset, image));
    }

    DynamicArray<GpuPBRMaterial> materials;
    for (fastgltf::Material& material : asset.materials) {
        materials.add(loadMaterial(textures, material));
    }

    DynamicArray<DynamicArray<uint32_t>> renderObjects;
    for (fastgltf::Mesh& mesh : asset.meshes) {
        loadMeshes(renderGroups, materials, asset, mesh, renderObjects, world);
    }

    if (!asset.animations.empty()) {
        uint32_t animationGroupID = world->getSystem<AnimationSystem>()->createGroup(assetName);

        uint32_t animationID = UINT32_MAX;
        for (fastgltf::Animation& animation : asset.animations) {
            uint32_t id = loadAnimation(animation, asset, rootNode, nodes, animationGroupID, world);
            if (animationID == UINT32_MAX) {
                animationID = id;
            }
        }
        if (animationID != UINT32_MAX) {
            root.addComponent(AnimationInstance{animationGroupID});
        }
    }

    DynamicArray<uint32_t> skins;
    for (fastgltf::Skin& skin : asset.skins) {
        skins.add(loadSkin(asset, skin, nodes, world));
    }

    for (uint32_t i = 0u; i < nodes.size(); i++) {
        const fastgltf::Node& node = asset.nodes[i];
        uint32_t entityID = nodes[i];

        if (node.cameraIndex.has_value()) {
            ecs.addComponent(entityID, cameras[node.cameraIndex.value()]);
        }

        if (node.lightIndex.has_value()) {
            GltfLight& light = lights[node.lightIndex.value()];
            switch (light.type) {
                case GltfLight::Type::DIRECTIONAL: {
                    ecs.addComponent(entityID, light.directional);
                    break;
                }
                case GltfLight::Type::SPOT: {
                    ecs.addComponent(entityID, light.spot);
                    break;
                }
                case GltfLight::Type::POINT: {
                    ecs.addComponent(entityID, light.point);
                    break;
                }
            }
        }

        if (node.skinIndex.has_value()) {
            ecs.addComponent(entityID, SkinInstance{skins[node.skinIndex.value()]});
        }

        if (node.meshIndex.has_value()) {
            DynamicArray<uint32_t>& objects = renderObjects[node.meshIndex.value()];
            for (uint32_t i = 0u; i < objects.size(); i++) {
                ecs.addComponent(entityID, RenderInstance{objects[i]});
            }
        }
    }

    return rootNode;
}