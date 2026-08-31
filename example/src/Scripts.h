#ifndef SCRIPTS_H_INCLUDED
#define SCRIPTS_H_INCLUDED

#pragma once

#include <cstdint>

#include <ScriptSystem.h>
#include <Transform3D.h>
#include <Viewport.h>
#include <input/Input.h>
#include <physics/3d/PhysicsObject3D.h>
#include <prvl.h>
#include <scene/3d/Scene3D.h>

struct PlayerScript {
    SCRIPT

    Entity root;

    Entity cameraPivot;

    float speed = 5.0f;
    float jumpForce = 5.0f;

    vec2 cameraAng;

    quat currentRotation = prvl::quat();

    bool jump = false;

    Viewport* viewport;

    void setup() {
        Scene3D* scene = world->getSystem<Scene3D>();
        uint32_t rootNode = scene->getRootNode();
        root = scene->getEntity(rootNode);

        uint32_t node = scene->getNode(entity);
        uint32_t pivotNode = scene->getChild(node, 0u);
        cameraPivot = scene->getEntity(pivotNode);
    }

    void physicsUpdate(double dt) {
        PhysicsObject3D obj;
        if (getComponent<PhysicsObject3D>(obj)) {
            obj.accY = -9.8;
            vec2 movement = prvl::vec2();
            if (Input::getKey(GLFW_KEY_W)) {
                movement.y = 1.0f;
            }
            if (Input::getKey(GLFW_KEY_S)) {
                movement.y = -1.0f;
            }
            if (Input::getKey(GLFW_KEY_D)) {
                movement.x = 1.0f;
            }
            if (Input::getKey(GLFW_KEY_A)) {
                movement.x = -1.0f;
            }
            if (dot(movement, movement) > 0.01f) {
                movement = normalize(movement);
            }
            vec2 joyDir = Input::getControllerJoystickPosition(0u, Joystick::LEFT);
            joyDir.y = -joyDir.y;
            movement += joyDir;
            if (dot(movement, movement) > 0.01f) {
                Transform3D* tx = cameraPivot.getComponentPtr<Transform3D>();
                mat3 basis = prvl::mat3(tx->global);
                vec3 forward = -basis[2];
                vec3 right = basis[0];
                forward.y = 0.0f;
                forward = normalize(forward);
                vec3 mov = movement.x * right + movement.y * forward;
                double mul = speed * dt;
                obj.prevPosX = obj.posX - mov.x * mul;
                obj.prevPosZ = obj.posZ - mov.z * mul;
            }
            if (jump) {
                obj.prevPosY -= jumpForce * dt;
                jump = false;
            }
            setComponent(obj);
        }
    }

    void frameUpdate(double dt) {
        vec2 stick = Input::getControllerJoystickPosition(0u, Joystick::RIGHT) * 2.0f;
        if (dot(stick, stick) > 0.01f) {
            cameraAng.x -= stick.y * dt;
            cameraAng.y += stick.x * dt;
        }

        quat targetRotation = prvl::quat({0.0f, 1.0f, 0.0f}, -cameraAng.y) * prvl::quat({1.0f, 0.0f, 0.0f}, cameraAng.x);

        float v = 1.0f - powf(0.000001f, dt);
        currentRotation = normalize(slerp(currentRotation, targetRotation, v));

        Transform3D* tx = cameraPivot.getComponentPtr<Transform3D>();
        mat4 rot = prvl::mat4(prvl::mat3(currentRotation)); // prvl::mat4(rotateY(cameraAng.y) * rotateX(cameraAng.x));
        rot[3] = tx->local[3];
        tx->local = rot;
        tx->dirtyLocal = true;
    }

    void input(const InputEvent& event) {
        if (event.type == KEY_EVENT && event.keyEvent.pressed && event.keyEvent.key == GLFW_KEY_SPACE) {
            jump = true;
        }
        if (event.type == CONTROLLER_BUTTON_EVENT && event.controllerButtonEvent.pressed && event.controllerButtonEvent.button == GLFW_GAMEPAD_BUTTON_A) {
            jump = true;
        }
        if (event.type != MOUSE_MOVE_EVENT) {
            return;
        }
        vec2 d = event.mouseMoveEvent.delta / prvl::vec2(viewport->size) * 2.0f;
        cameraAng.x -= d.y;
        cameraAng.y += d.x;
    }
};
REGISTER_SCRIPT(PlayerScript)

#endif