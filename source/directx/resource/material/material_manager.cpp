#include "material_manager.h"

MaterialManager::MaterialManager()
{
}

MaterialManager::~MaterialManager()
{
}

Material* MaterialManager::GetMaterial(std::string name) const
{
	auto it = m_MaterialMap.find(name);

	if (it == m_MaterialMap.end())
	{
		return nullptr;
	}

	return it->second.get();
}
