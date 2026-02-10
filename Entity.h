#pragma once
#include "Mesh.h"
#include "Transform.h"
#include <memory>
class Entity
{
public:
	Entity(std::shared_ptr<Mesh> mesh);
	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();

private:
	std::shared_ptr<Mesh> mesh;
	Transform transform;
};

