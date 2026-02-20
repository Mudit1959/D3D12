#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> inMesh, std::shared_ptr<Material> inMaterial) 
{
	mesh = inMesh;
	material = inMaterial;
}

void Entity::SetMaterial(std::shared_ptr<Material> m) { material = m; }

std::shared_ptr<Mesh> Entity::GetMesh() { return mesh; }
std::shared_ptr<Material> Entity::GetMaterial() { return material; }
Transform* Entity::GetTransform() { return &transform;  }