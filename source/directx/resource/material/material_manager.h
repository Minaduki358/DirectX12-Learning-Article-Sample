#pragma once

#include"material.h"

class MaterialManager
{
public:
	MaterialManager();
	~MaterialManager();

	template<typename T>
	void CreateMaterialMap(std::string materialName, T material)
	{
		static_assert(std::is_base_of<Material, T>::value, "T must inherit from Material");
		auto material = std::make_unique<T>();
		m_MaterialMap.insert(materialName, material);
	}

	Material* GetMaterial(std::string materialName) const;

private:
	std::unordered_map<std::string, std::unique_ptr<Material>> m_MaterialMap;
};