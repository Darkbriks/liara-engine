//
// Created by antoi on 20/10/2024.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Liara
{
    class GravityPhysicsSystem
    {
    public:
        explicit GravityPhysicsSystem(const float strength) : strengthGravity{strength} {}

        const float strengthGravity;

        // dt stands for delta time, and specifies the amount of time to advance the simulation
        // substeps is how many intervals to divide the forward time step in. More substeps result in a
        // more stable simulation, but takes longer to compute
        void Update(std::vector<Liara_GameObject>& objs, const float dt, unsigned int substeps=1)
        {
            const float stepDelta = dt / static_cast<float>(substeps);
            for (int i = 0; i < substeps; i++) { StepSimulation(objs, stepDelta); }
        }

        [[nodiscard]] glm::vec2 ComputeForce(const Liara_GameObject& fromObj, const Liara_GameObject& toObj) const
        {
            const auto offset = fromObj.m_Transform.position - toObj.m_Transform.position;
            const float distanceSquared = glm::dot(offset, offset);

            if (glm::abs(distanceSquared) < 1e-10f) { return {.0f, .0f}; }

            const float force = strengthGravity * toObj.m_RigidBody.mass * fromObj.m_RigidBody.mass / distanceSquared;
            return force * offset / glm::sqrt(distanceSquared);
        }

    private:
        void StepSimulation(std::vector<Liara_GameObject>& physicsObjs, const float dt) const
        {
            for (auto iterA = physicsObjs.begin(); iterA != physicsObjs.end(); ++iterA)
            {
                auto& objA = *iterA;
                for (auto iterB = iterA; iterB != physicsObjs.end(); ++iterB)
                {
                    if (iterA == iterB) continue;
                    auto& objB = *iterB;

                    auto force = ComputeForce(objA, objB);
                    objA.m_RigidBody.velocity += dt * -force / objA.m_RigidBody.mass;
                    objB.m_RigidBody.velocity += dt * force / objB.m_RigidBody.mass;
                }
            }

            // update each objects position based on its final velocity
            for (auto& obj : physicsObjs) { obj.m_Transform.position += dt * obj.m_RigidBody.velocity; }
        }
    };

    class Vec2FieldSystem
    {
    public:
        void Update(const GravityPhysicsSystem& physicsSystem, std::vector<Liara_GameObject>& physicsObjs, std::vector<Liara_GameObject>& vectorField)
        {
            // For each field line we caluclate the net graviation force for that point in space
            for (auto& vf : vectorField)
            {
                glm::vec2 direction{};
                for (auto& obj : physicsObjs) { direction += physicsSystem.ComputeForce(obj, vf); }

                // This scales the length of the field line based on the log of the length
                // values were chosen just through trial and error based on what i liked the look
                // of and then the field line is rotated to point in the direction of the field
                vf.m_Transform.scale.x = 0.005f + 0.045f * glm::clamp(glm::log(glm::length(direction) + 1) / 3.f, 0.f, 1.f);
                vf.m_Transform.rotation = static_cast<float>(atan2(direction.y, direction.x));
            }
        }
    };

    inline std::unique_ptr<Liara_Model> CreateSquareModel(Liara_Device& device, const glm::vec2 offset)
    {
        std::vector<Liara_Model::Vertex> vertices = {
            {{-0.5f, -0.5f}},
            {{0.5f, 0.5f}},
            {{-0.5f, 0.5f}},
            {{-0.5f, -0.5f}},
            {{0.5f, -0.5f}},
            {{0.5f, 0.5f}},
        };
        for (auto&[position, color] : vertices) { position += offset; }
        return std::make_unique<Liara_Model>(device, vertices);
    }

    inline std::unique_ptr<Liara_Model> CreateCircleModel(Liara_Device& device, const unsigned int numSides)
    {
        std::vector<Liara_Model::Vertex> uniqueVertices{};
        for (int i = 0; i < numSides; i++)
        {
            const float angle = static_cast<float>(i) * glm::two_pi<float>() / static_cast<float>(numSides);
            uniqueVertices.push_back({{glm::cos(angle), glm::sin(angle)}});
        }
        uniqueVertices.push_back({});  // adds center vertex at 0, 0

        std::vector<Liara_Model::Vertex> vertices{};
        for (int i = 0; i < numSides; i++)
        {
            vertices.push_back(uniqueVertices[i]);
            vertices.push_back(uniqueVertices[(i + 1) % numSides]);
            vertices.push_back(uniqueVertices[numSides]);
        }
        return std::make_unique<Liara_Model>(device, vertices);
    }
}