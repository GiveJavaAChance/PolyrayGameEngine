#include <cstdint>
#include <iostream>
#include <fstream>

#include <base/glad/glad.h>
#include <GLFW/glfw3.h>

#include <GLFWindow.h>
#include <prvl.h>
#include <parsing/Json.h>
#include <structure/SVO.h>
#include <shader/ShaderManager.h>
#include <shader/ShaderProgram.h>
#include <rendering/ShaderBuffer.h>
#include <rendering/GLGBuffer.h>
#include <BindingRegistry.h>
#include <utils/perf.h>
#include <input/Input.h>


struct SOTFElement {
    uint32_t profileID;
    vec3 pos;
    quat rot;
    float lengthScale = 1.0f;
};

struct SOTFStructureLink {
    uint32_t typeID;
    uint32_t InstanceID;
};

struct SOTFStructure {
    uint32_t typeID;
    vec3 pos;
    quat rot;
    SOTFStructureLink* linkedStructures = nullptr;
    uint32_t linkedStructureCount;
    SOTFElement* elements = nullptr;
    uint32_t elementCount;

    static SOTFStructure** load(JSONArray structures, uint32_t& out_structureGroupCount, uint32_t*& out_groupSizes) {
        SOTFStructure** out = new SOTFStructure*[structures.size()];
        out_structureGroupCount = structures.size();
        out_groupSizes = new uint32_t[structures.size()];
        for (uint32_t i = 0u; i < structures.size(); i++) {
            JSONValue& value = structures[i];
            out[i] = nullptr;
            if (value.type == JSON_ARRAY) {
                JSONArray& structure = value.arr;
                SOTFStructure* col = new SOTFStructure[structure.size()];
                out[i] = col;
                out_groupSizes[i] = structure.size();
                for (uint32_t j = 0u; j < structure.size(); j++) {
                    JSONObject& structureData = structure[j].obj;
                    SOTFStructure& obj = col[j];
                    obj.typeID = static_cast<uint32_t>(structureData["TypeID"].i);

                    JSONObject& position = structureData["Position"].obj;
                    obj.pos = prvl::vec3(position["x"].f, position["y"].f, position["z"].f);

                    JSONObject& rotation = structureData["Rotation"].obj;
                    if (rotation.contains("x")) {
                        obj.rot.x = rotation["x"].f;
                    }
                    if (rotation.contains("y")) {
                        obj.rot.y = rotation["y"].f;
                    }
                    if (rotation.contains("z")) {
                        obj.rot.z = rotation["z"].f;
                    }
                    if (rotation.contains("w")) {
                        obj.rot.w = rotation["w"].f;
                    }

                    JSONArray& linkedStructures = structureData["LinkedStructures"].arr;
                    obj.linkedStructures = new SOTFStructureLink[linkedStructures.size()];
                    obj.linkedStructureCount = linkedStructures.size();
                    for (uint32_t k = 0u; k < linkedStructures.size(); k++) {
                        JSONValue& value = linkedStructures[k];
                        if (value.type == JSON_OBJECT) {
                            JSONObject& link = value.obj;
                            uint32_t instanceID = 0u;
                            if (link.contains("InstanceID")) {
                                instanceID = static_cast<uint32_t>(link["InstanceID"].i);
                            }
                            obj.linkedStructures[k] = SOTFStructureLink{static_cast<uint32_t>(link["TypeID"].i), instanceID};
                        } else {
                            obj.linkedStructures[k] = SOTFStructureLink{0xFFFFFFFFu, 0xFFFFFFFFu};
                        }
                    }

                    JSONArray& elements = structureData["Elements"].arr;
                    obj.elements = new SOTFElement[elements.size()];
                    obj.elementCount = elements.size();
                    for (uint32_t k = 0u; k < elements.size(); k++) {
                        JSONObject& element = elements[k].obj;
                        SOTFElement& e = obj.elements[k];
                        e.profileID = static_cast<uint32_t>(element["ProfileID"].i);
                        JSONObject& ePosition = element["Position"].obj;
                        e.pos = prvl::vec3(ePosition["x"].f, ePosition["y"].f, ePosition["z"].f);

                        JSONObject& eRotation = element["Rotation"].obj;
                        if (eRotation.contains("x")) {
                            e.rot.x = eRotation["x"].f;
                        }
                        if (eRotation.contains("y")) {
                            e.rot.y = eRotation["y"].f;
                        }
                        if (eRotation.contains("z")) {
                            e.rot.z = eRotation["z"].f;
                        }
                        if (eRotation.contains("w")) {
                            e.rot.w = eRotation["w"].f;
                        }
                        e.lengthScale = element["LengthScale"].f;
                    }
                }
            }
        }
        return out;
    }
};

namespace SOTFStructureVoxelizer {
    static SVO voxelize(SOTFStructure** structures, uint32_t structureGroupCount, uint32_t* groupSizes, uint32_t svoSize) {
        vec3 bardMin = prvl::vec3(1000000.0f, 1000000.0f, 1000000.0f);
        vec3 bardMax = prvl::vec3(-1000000.0f, -1000000.0f, -1000000.0f);

        float clipMinX = 1200.557f;
        float clipMinY = 100.6582f;
        float clipMinZ = 1100.8665f;
        float clipMaxX = 1300.7844f;
        float clipMaxY = 400.2613f;
        float clipMaxZ = 1300.7334f;
        std::vector<SOTFElement*> logs;
        for (uint32_t i = 0u; i < structureGroupCount; i++) {
            SOTFStructure* structure = structures[i];
            if (structure != nullptr) {
                uint32_t size = groupSizes[i];
                for (uint32_t j = 0u; j < size; j++) {
                    SOTFStructure& s = structure[j];
                    for (uint32_t k = 0u; k < s.elementCount; k++) {
                        SOTFElement& e = s.elements[k];
                        if (e.pos.x < clipMinX || e.pos.y < clipMinY || e.pos.z < clipMinZ || e.pos.x > clipMaxX || e.pos.y > clipMaxY || e.pos.z > clipMaxZ) {
                            continue;
                        }
                        bardMin = min(bardMin, e.pos);
                        bardMax = max(bardMax, e.pos);
                        logs.push_back(&e);
                    }
                }
            }
        }

        vec3 d = -bardMin;
        bardMax += d;
        float maxSize = max(max(bardMax.x, bardMax.y), bardMax.z);

        std::cout << "Element count: " << logs.size() << std::endl;

        SVOBuilder b(svoSize);
        float scale = static_cast<float>(b.dim) / maxSize;
        for (SOTFElement* log : logs) {
            uint32_t profileID = log->profileID;
            float lengthMul = 1.2f;
            switch (profileID) {
                case 42:
                    lengthMul = 0.3f;
                    break;
                case 7:
                case 66:
                    lengthMul = 0.4f;
                    break;
                case 151:
                    lengthMul = 0.5f;
                    break;
                case 33:
                    lengthMul = 0.6f;
                    break;
                case 8:
                    lengthMul = 0.7f;
                    break;
                case 9:
                case 34:
                    lengthMul = 0.9f;
                    break;
                case 2:
                    lengthMul = 1.3f;
                    break;
            }
            float offset = 0.5f;
            switch (profileID) {
                case 33:
                case 34:
                    offset = 0.0f;
                    break;
                case 66:
                    offset = 1.0f;
                    break;
            }
            int matIdx = profileID % 256;
            bool isHalf = profileID == 6 || profileID == 152 || profileID == 314 || profileID == 315 || profileID == 323;
            log->pos += d;
            log->pos *= scale;

            mat3 basis = mat3(log->rot);

            float radius = scale * (profileID == 151 ? 0.05f : 0.2f);
            float len = log->lengthScale * scale * 2.0f * lengthMul;

            vec3& dd = basis[2];

            vec3 s = log->pos - offset * len * dd;

            vec3 e = radius * sqrt(max(prvl::vec3(0.0f), 1.0f - dd));

            vec3 e2 = s + dd * len;

            uvec3 minS = prvl::uvec3(clamp(prvl::ivec3(floor(min(s, e2) - e)), prvl::ivec3(0), prvl::ivec3(b.dim - 1)));
            uvec3 maxS = prvl::uvec3(clamp(prvl::ivec3(ceil(max(s, e2) + e)), prvl::ivec3(0), prvl::ivec3(b.dim - 1)));

            for (uint32_t x = minS.x; x <= maxS.x; x++) {
                for (uint32_t y = minS.y; y <= maxS.y; y++) {
                    for (uint32_t z = minS.z; z <= maxS.z; z++) {

                        vec3 pos = prvl::vec3(x, y, z);
                        vec3 p = pos - s;

                        float t = dot(p, dd);

                        if (t < 0 || t > len || (isHalf && dot(p, basis[1]) > 0.0f)) {
                            continue;
                        }

                        vec3 c = s + dd * t;
                        vec3 del = pos - c;
                        float distSq = dot(del, del);
                        if (distSq <= radius * radius) {
                            b.setVoxel(x, y, z, matIdx);
                        }
                    }
                }
            }
        }
        return b.build();
    }
}

uint64_t countVoxels(Node& node, DynamicArray<Node>& nodes) {
    uint32_t childCount = __builtin_popcountll(node.mask());
    if(node.isLeaf()) {
        return static_cast<uint64_t>(childCount);
    }
    uint64_t count = 0ull;
    uint32_t ptr = node.ptr();
    for(uint32_t i = 0u; i < childCount; i++) {
        count += countVoxels(nodes[ptr + i], nodes);
    }
    return count;
}

struct Transform {
    mat3x4 rot;
    vec3 pos;
    float __padding__0;
};

struct Instance {
    uint32_t modelIdx;
    uint32_t transformIdx;
};

struct GeometryPass {
    ShaderBuffer nodeBuffer;
    ShaderBuffer modelBuffer;
    ShaderBuffer transformBuffer;
    ShaderBuffer instanceBuffer;
    ShaderBuffer leafBuffer;
    ShaderBuffer materialBuffer;

    ShaderProgram shader;

    GLuint vao;

    GLuint query;
    uint64_t time;

    GeometryPass() : nodeBuffer(GL_STATIC_DRAW), modelBuffer(GL_STATIC_DRAW), transformBuffer(GL_STATIC_DRAW), instanceBuffer(GL_STATIC_DRAW), leafBuffer(GL_STATIC_DRAW), materialBuffer(GL_STATIC_DRAW) {
        ShaderManager::setValue("NODE_IDX", BindingRegistry::bindBufferBase(nodeBuffer, GL_SHADER_STORAGE_BUFFER));
        ShaderManager::setValue("MODEL_IDX", BindingRegistry::bindBufferBase(modelBuffer, GL_SHADER_STORAGE_BUFFER));
        ShaderManager::setValue("TRANSFORM_IDX", BindingRegistry::bindBufferBase(transformBuffer, GL_SHADER_STORAGE_BUFFER));
        ShaderManager::setValue("INSTANCE_IDX", BindingRegistry::bindBufferBase(instanceBuffer, GL_SHADER_STORAGE_BUFFER));
        ShaderManager::setValue("LEAF_IDX", BindingRegistry::bindBufferBase(leafBuffer, GL_SHADER_STORAGE_BUFFER));
        ShaderManager::setValue("MAT_IDX", BindingRegistry::bindBufferBase(materialBuffer, GL_SHADER_STORAGE_BUFFER));

        uint32_t vert = ShaderManager::compileShaderFile("Quad.vert", GL_VERTEX_SHADER, true);
        uint32_t frag = ShaderManager::compileShaderFile("GeometryPass.frag", GL_FRAGMENT_SHADER, true);
        shader = ShaderManager::createProgram({vert, frag});
        float quadVertices[]{
            -1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f
        };
        ShaderBuffer quadVbo(GL_STATIC_DRAW);
        quadVbo.uploadData(quadVertices, 8u);

        vao = ShaderManager::createVAO(shader, {quadVbo.ID});

        glGenQueries(1, &query);
    }

    void uploadModels(SVO* svos, uint32_t count) {
        uint32_t totalNodeCount = 0u;
        for (uint32_t i = 0u; i < count; i++) {
            totalNodeCount += svos[i].nodes.size();
        }
        nodeBuffer.setSize(totalNodeCount * sizeof(Node));
        uint32_t* pointers = alloc<uint32_t>(count);
        uint32_t idx = 0u;
        for(uint32_t i = 0u; i < count; i++) {
            pointers[i] = idx / 3u;
            SVO& svo = svos[i];
            nodeBuffer.uploadPartialData(svo.nodes.data(), svo.nodes.size(), idx);
            idx += svo.nodes.size();
        }
        uint32_t totalLeafCount = 0u;
        for (uint32_t i = 0u; i < count; i++) {
            totalLeafCount += svos[i].leafData.size();
        }
        uint32_t leafDataSize = (totalLeafCount + 3u) >> 2u;
        uint32_t* leafData = alloc<uint32_t>(leafDataSize);
        std::memset(leafData, 0u, leafDataSize * sizeof(uint32_t));
        idx = 0u;
        uint32_t* leafPtrs = alloc<uint32_t>(count);
        for (uint32_t i = 0u; i < count; i++) {
            SVO& svo = svos[i];
            leafPtrs[i] = idx;
            for (uint32_t j = 0u; j < svo.leafData.size(); j++) {
                leafData[idx >> 2u] |= svo.leafData[j] << ((idx & 3u) << 3u);
                idx++;
            }
        }
        leafBuffer.uploadData(leafData, leafDataSize);
        free(leafData);
        uint32_t* modelData = alloc<uint32_t>(count << 2u);
        for (uint32_t i = 0u; i < count; i++) {
            SVO& svo = svos[i];
            modelData[i * 4u] = svo.size;
            modelData[i * 4u + 1u] = svo.dim;
            modelData[i * 4u + 2u] = pointers[i];
            modelData[i * 4u + 3u] = leafPtrs[i];
        }
        modelBuffer.uploadData(modelData, count << 2u);
        free(pointers);
        free(leafPtrs);
        free(modelData);
    }

    void uploadTransforms(Transform* transforms, uint32_t count) {
        transformBuffer.uploadData(transforms, count);
    }

    void uploadInstances(Instance* instances, uint32_t count) {
        instanceBuffer.uploadData(instances, count);
    }

    void uploadMaterial(vec4* paletteData, uint32_t count) {
        materialBuffer.uploadData(paletteData, count);
    }

    void render() {
        glBeginQuery(GL_TIME_ELAPSED, query);
        shader.use();
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glEndQuery(GL_TIME_ELAPSED);
        glGetQueryObjectui64v(query, GL_QUERY_RESULT, &time);
    }
};

struct LightingPass {

    GLTexture* albedo;
    GLTexture* normalMetal;
    GLTexture* depth;

    ShaderProgram shader;

    GLuint vao;

    LightingPass(GLTexture* albedo, GLTexture* normalMetal, GLTexture* depth) : albedo(albedo), normalMetal(normalMetal), depth(depth) {
        uint32_t vert = ShaderManager::compileShaderFile("Quad.vert", GL_VERTEX_SHADER, true);
        uint32_t frag = ShaderManager::compileShaderFile("LightingPass.frag", GL_FRAGMENT_SHADER, true);
        shader = ShaderManager::createProgram({vert, frag});

        float quadVertices[]{
            -1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f
        };
        ShaderBuffer quadVbo(GL_STATIC_DRAW);
        quadVbo.uploadData(quadVertices, 8u);

        vao = ShaderManager::createVAO(shader, {quadVbo.ID});
    }

    void render() {
        shader.use();
        glBindTextureUnit(0, albedo->ID);
        glBindTextureUnit(1, normalMetal->ID);
        glBindTextureUnit(2, depth->ID);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
};

constexpr uint32_t PALETTE[]{
    0x00000000u, 0xFFFFFFFFu, 0xFFCCFFFFu, 0xFF99FFFFu, 0xFF66FFFFu, 0xFF33FFFFu,
    0xFF00FFFFu, 0xFFFFCCFFu, 0xFFCCCCFFu, 0xFF99CCFFu, 0xFF66CCFFu, 0xFF33CCFFu,
    0xFF00CCFFu, 0xFFFF99FFu, 0xFFCC99FFu, 0xFF9999FFu, 0xFF6699FFu, 0xFF3399FFu,
    0xFF0099FFu, 0xFFFF66FFu, 0xFFCC66FFu, 0xFF9966FFu, 0xFF6666FFu, 0xFF3366FFu,
    0xFF0066FFu, 0xFFFF33FFu, 0xFFCC33FFu, 0xFF9933FFu, 0xFF6633FFu, 0xFF3333FFu,
    0xFF0033FFu, 0xFFFF00FFu, 0xFFCC00FFu, 0xFF9900FFu, 0xFF6600FFu, 0xFF3300FFu,
    0xFF0000FFu, 0xFFFFFFCCu, 0xFFCCFFCCu, 0xFF99FFCCu, 0xFF66FFCCu, 0xFF33FFCCu,
    0xFF00FFCCu, 0xFFFFCCCCu, 0xFFCCCCCCu, 0xFF99CCCCu, 0xFF66CCCCu, 0xFF33CCCCu,
    0xFF00CCCCu, 0xFFFF99CCu, 0xFFCC99CCu, 0xFF9999CCu, 0xFF6699CCu, 0xFF3399CCu,
    0xFF0099CCu, 0xFFFF66CCu, 0xFFCC66CCu, 0xFF9966CCu, 0xFF6666CCu, 0xFF3366CCu,
    0xFF0066CCu, 0xFFFF33CCu, 0xFFCC33CCu, 0xFF9933CCu, 0xFF6633CCu, 0xFF3333CCu,
    0xFF0033CCu, 0xFFFF00CCu, 0xFFCC00CCu, 0xFF9900CCu, 0xFF6600CCu, 0xFF3300CCu,
    0xFF0000CCu, 0xFFFFFF99u, 0xFFCCFF99u, 0xFF99FF99u, 0xFF66FF99u, 0xFF33FF99u,
    0xFF00FF99u, 0xFFFFCC99u, 0xFFCCCC99u, 0xFF99CC99u, 0xFF66CC99u, 0xFF33CC99u,
    0xFF00CC99u, 0xFFFF9999u, 0xFFCC9999u, 0xFF999999u, 0xFF669999u, 0xFF339999u,
    0xFF009999u, 0xFFFF6699u, 0xFFCC6699u, 0xFF996699u, 0xFF666699u, 0xFF336699u,
    0xFF006699u, 0xFFFF3399u, 0xFFCC3399u, 0xFF993399u, 0xFF663399u, 0xFF333399u,
    0xFF003399u, 0xFFFF0099u, 0xFFCC0099u, 0xFF990099u, 0xFF660099u, 0xFF330099u,
    0xFF000099u, 0xFFFFFF66u, 0xFFCCFF66u, 0xFF99FF66u, 0xFF66FF66u, 0xFF33FF66u,
    0xFF00FF66u, 0xFFFFCC66u, 0xFFCCCC66u, 0xFF99CC66u, 0xFF66CC66u, 0xFF33CC66u,
    0xFF00CC66u, 0xFFFF9966u, 0xFFCC9966u, 0xFF999966u, 0xFF669966u, 0xFF339966u,
    0xFF009966u, 0xFFFF6666u, 0xFFCC6666u, 0xFF996666u, 0xFF666666u, 0xFF336666u,
    0xFF006666u, 0xFFFF3366u, 0xFFCC3366u, 0xFF993366u, 0xFF663366u, 0xFF333366u,
    0xFF003366u, 0xFFFF0066u, 0xFFCC0066u, 0xFF990066u, 0xFF660066u, 0xFF330066u,
    0xFF000066u, 0xFFFFFF33u, 0xFFCCFF33u, 0xFF99FF33u, 0xFF66FF33u, 0xFF33FF33u,
    0xFF00FF33u, 0xFFFFCC33u, 0xFFCCCC33u, 0xFF99CC33u, 0xFF66CC33u, 0xFF33CC33u,
    0xFF00CC33u, 0xFFFF9933u, 0xFFCC9933u, 0xFF999933u, 0xFF669933u, 0xFF339933u,
    0xFF009933u, 0xFFFF6633u, 0xFFCC6633u, 0xFF996633u, 0xFF666633u, 0xFF336633u,
    0xFF006633u, 0xFFFF3333u, 0xFFCC3333u, 0xFF993333u, 0xFF663333u, 0xFF333333u,
    0xFF003333u, 0xFFFF0033u, 0xFFCC0033u, 0xFF990033u, 0xFF660033u, 0xFF330033u,
    0xFF000033u, 0xFFFFFF00u, 0xFFCCFF00u, 0xFF99FF00u, 0xFF66FF00u, 0xFF33FF00u,
    0xFF00FF00u, 0xFFFFCC00u, 0xFFCCCC00u, 0xFF99CC00u, 0xFF66CC00u, 0xFF33CC00u,
    0xFF00CC00u, 0xFFFF9900u, 0xFFCC9900u, 0xFF999900u, 0xFF669900u, 0xFF339900u,
    0xFF009900u, 0xFFFF6600u, 0xFFCC6600u, 0xFF996600u, 0xFF666600u, 0xFF336600u,
    0xFF006600u, 0xFFFF3300u, 0xFFCC3300u, 0xFF993300u, 0xFF663300u, 0xFF333300u,
    0xFF003300u, 0xFFFF0000u, 0xFFCC0000u, 0xFF990000u, 0xFF660000u, 0xFF330000u,
    0xFF0000EEu, 0xFF0000DDu, 0xFF0000BBu, 0xFF0000AAu, 0xFF000088u, 0xFF000077u,
    0xFF000055u, 0xFF000044u, 0xFF000022u, 0xFF000011u, 0xFF00EE00u, 0xFF00DD00u,
    0xFF00BB00u, 0xFF00AA00u, 0xFF008800u, 0xFF007700u, 0xFF005500u, 0xFF004400u,
    0xFF002200u, 0xFF001100u, 0xFFEE0000u, 0xFFDD0000u, 0xFFBB0000u, 0xFFAA0000u,
    0xFF880000u, 0xFF770000u, 0xFF550000u, 0xFF440000u, 0xFF220000u, 0xFF110000u,
    0xFFEEEEEEu, 0xFFDDDDDDu, 0xFFBBBBBBu, 0xFFAAAAAAu, 0xFF888888u, 0xFF777777u,
    0xFF555555u, 0xFF444444u, 0xFF222222u, 0xFF111111u
};


struct Camera3D {
    mat4 cameraTransform;
    mat4 inverseCameraTransform;
    mat4 projection;
    mat4 inverseProjection;
    vec3 cameraPos;
    float __padding__0;
};

struct Environment {
    vec3 sunDir;
    float __padding__0;
    vec3 sunColor;
    float __padding__1;
    vec3 ambientColor;
    float __padding__2;
};

Camera3D cam;
Environment env;

GLFWindow w("testing");

int main() {
    ResourceManager::addLocalResource("res/shaders");
    ResourceManager::addLocalResource("res/shaders/voxel raytracing");
    ResourceManager::addLocalResource("res/structures");

    w.createFrame(500u, 500u, false, true, false);
    ShaderManager::setValue("WIDTH", w.getWidth());
    ShaderManager::setValue("HEIGHT", w.getHeight());

    cam.projection = reverseZPerspectiveProjection(1.57079632679f, static_cast<float>(w.getWidth()) / static_cast<float>(w.getHeight()), 0.1f);
    cam.projection.data()[14] = -0.1f;
    cam.inverseProjection = inverse(cam.projection);

    ShaderBuffer cameraBuffer(GL_DYNAMIC_DRAW);
    ShaderBuffer envBuffer(GL_DYNAMIC_DRAW);
    cameraBuffer.setSize(sizeof(Camera3D));
    envBuffer.setSize(sizeof(Environment));

    env.sunColor = prvl::vec3(1.0f, 1.0f, 0.5f);
    env.ambientColor = prvl::vec3(0.2f, 0.5f, 1.0f);
    env.sunDir = normalize(prvl::vec3(1.0f));

    envBuffer.uploadPartialData(&env, 1, 0);

    ShaderManager::setValue("CAM3D_IDX", BindingRegistry::bindBufferBase(cameraBuffer, GL_UNIFORM_BUFFER));
    ShaderManager::setValue("ENV_IDX", BindingRegistry::bindBufferBase(envBuffer, GL_UNIFORM_BUFFER));

    ShaderBuffer traceBuffer(GL_STATIC_DRAW);
    traceBuffer.setSize(w.getWidth() * w.getHeight() * 3 * 4);
    ShaderManager::setValue("TRACE_IDX", BindingRegistry::bindBufferBase(traceBuffer, GL_SHADER_STORAGE_BUFFER));

    cam.cameraPos.x = -27.576263f;
    cam.cameraPos.y = 142.1592f;
    cam.cameraPos.z = 320.67758f;
    vec3 cameraAng;
    cameraAng.x = -0.14814958f;
    cameraAng.y = -0.8112563f;
    cameraAng.z = 0.0f;
    cameraBuffer.uploadPartialData(&cam, 1u, 0u);

    GLTexture albedo = GLTexture::createTexture2D(w.getWidth(), w.getHeight(), GL_RGBA16F);
    GLTexture normalMetal = GLTexture::createTexture2D(w.getWidth(), w.getHeight(), GL_RGBA16F);
    GLTexture depth = GLTexture::createTexture2D(w.getWidth(), w.getHeight(), GL_DEPTH_COMPONENT32);
    GLGBuffer geometryBuffer({&albedo, &normalMetal}, &depth);

    GeometryPass geometryPass;
    LightingPass lightingPass(&albedo, &normalMetal, &depth);

    std::string data = ResourceManager::getResourceAsString("structure.json");
    const char* json = data.c_str();

    uint64_t start = Time::nanoTime();
    JSONValue root = JSON::parseJSON(json);
    uint64_t end = Time::nanoTime();
    std::cout << "Parse Time: " << (end - start) / 1000000 << std::endl;
    if(root.type != JSON_OBJECT) {
        std::cout << "JSON is not a object." << std::endl;
        std::cout << static_cast<int>(root.type) << std::endl;
        return 0;
    }
    JSONObject& obj = root.obj;
    if(!obj.contains("Structures")) {
        std::cout << "JSON does not contain a \"Structures\" key." << std::endl;
        return 0;
    }
    JSONValue& value = obj["Structures"];
    if(value.type != JSON_ARRAY) {
        std::cout << "\"Structures\" is not a array." << std::endl;
        return 0;
    }
    JSONArray& structuresJson = value.arr;
    uint32_t structureGroupCount = 0u;
    uint32_t* groupSizes = nullptr;
    start = Time::nanoTime();
    SOTFStructure** structures = SOTFStructure::load(structuresJson, structureGroupCount, groupSizes);
    end = Time::nanoTime();
    std::cout << "Load Time: " << (end - start) / 1000000 << std::endl;
    start = Time::nanoTime();
    SVO svo = SOTFStructureVoxelizer::voxelize(structures, structureGroupCount, groupSizes, 5u);
    end = Time::nanoTime();
    std::cout << "Voxelize Time: " << (end - start) / 1000000 << std::endl;
    std::cout << "Size: " << svo.size << ", Dim: " << svo.dim << ", Count: " << countVoxels(svo.nodes[0u], svo.nodes) << std::endl;

    vec4 materialPalette[256u];
    for (uint32_t i = 0u; i < 256u; i++) {
        uint32_t c = PALETTE[i];
        if(i == 1u) {
            c = 0xFFAAAAAAu;
        }
        materialPalette[i] = prvl::vec4(prvl::vec3(prvl::uvec3(c >> 16, c >> 8, c) & 0xFFu) / 255.0f, 0.7f);
    }

    geometryPass.uploadMaterial(materialPalette, 256u);

    geometryPass.uploadModels(&svo, 1u);

    constexpr uint32_t amt = 1u;
    Transform transforms[amt];
    Instance instances[amt];
    for (uint32_t i = 0u; i < amt; i++) {
        transforms[i] = Transform{
            mat3x4(diag(prvl::vec3(1.0f / 16.0f))),
            prvl::vec3(0.0f)
        };
        instances[i] = Instance{i, i};
    }
    geometryPass.uploadInstances(instances, amt);
    geometryPass.uploadTransforms(transforms, amt);


    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0);

    double dt = 1.0 / 165.0;
    double time = 0.0;
    w.hideCursor(true);
    for(uint32_t frame = 0u; w.isWindowOpen(); frame++) {
        uint64_t startTime = Time::nanoTime();

        double px, py;
        w.getMousePos(px, py);
        float cx = w.getWidth() * 0.5f;
        float cy = w.getHeight() * 0.5f;
        float dx = (static_cast<float>(px) - cx) / cx;
        float dy = (static_cast<float>(py) - cy) / cy;
        cameraAng.x += dy;
        cameraAng.y -= dx;
        cam.cameraTransform = mat4(rotateX(cameraAng.x) * rotateY(cameraAng.y));
        cam.inverseCameraTransform = transpose(cam.cameraTransform);
        w.setMousePos(cx, cy);

        vec3 movement = prvl::vec3();
        vec3 forward = -prvl::vec3(cam.inverseCameraTransform[2]);
        if(Input::getKey(GLFW_KEY_W)) {
            movement += forward;
        }
        if(Input::getKey(GLFW_KEY_S)) {
            movement -= forward;
        }
        if(Input::getKey(GLFW_KEY_A)) {
            movement -= prvl::vec3(cam.inverseCameraTransform[0]);
        }
        if(Input::getKey(GLFW_KEY_D)) {
            movement += prvl::vec3(cam.inverseCameraTransform[0]);
        }
        if(dot(movement, movement) > 0.01f) {
            double mul = 2.0 * dt / length(movement);
            if(Input::getKey(GLFW_KEY_LEFT_ALT)) {
                mul *= 10.0;
            }
            cam.cameraPos += movement * static_cast<float>(mul);
        }

        cameraBuffer.uploadPartialData(&cam, 1, 0);

        lightingPass.shader.use();
        lightingPass.shader.setUniform("time", (float) time);
        geometryPass.shader.use();
        geometryPass.shader.setUniform("time", (float) time);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        geometryBuffer.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        geometryPass.render();
        geometryBuffer.unbind();
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        lightingPass.render();

        w.update();
        dt = static_cast<double>(Time::nanoTime() - startTime) / 1000000000.0;
        time += dt;
    }
}
