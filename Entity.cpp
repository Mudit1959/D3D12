#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> mesh) 
{
}

std::shared_ptr<Mesh> Entity::GetMesh() { return mesh; }
Transform* Entity::GetTransform() { return &transform;  }