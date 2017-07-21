#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <map>
#include <iostream>
#include "resource.h"
#include "logger.h"

static const std::string MODELS_PATH{ "data/models/" };
static const std::string TEXTURE_PATH{ "data/textures/" };
static const std::string CONFIGURATION_PATH{ "config/" };
static const std::string AUDIO_PATH{ "data/audio/" };

static int s_Id{ 0 };

/**
 * \brief Resource manager class of the engine. Manages several types of resources: models, textures, configuration
 * and audio.
 */
class ResourceManager {
private:
	/**
	 * \brief Correlate resource by names.
	 */
	std::map<std::string, Resource*> m_ResourcesByName;

	/**
	 * \brief Correlate resources by indices.
	 */
	std::map<unsigned int, Resource*> m_ResourcesById;

public:
	~ResourceManager()
	{
		for (auto resource : m_ResourcesByName) {
			delete resource.second;
		}

		m_ResourcesByName.clear();

		m_ResourcesById.clear();
	}

	/**
	 * \brief Load a new resource by file name
	 * \return true if the resource is loaded correctly,
	 * false otherwise.
	 */
	template<typename T, typename... Args>
	bool Load(const std::string& fileName, Args&&...args)
	{
		T* resource{ new T{std::forward<Args>(args)...} };

		if (resource->Load(fileName)) {
			resource->SetId(s_Id);
			RegisterResource(resource, fileName);
			LOG("Loaded ->  " + fileName);
			return true;
		}

		return false;
	}

	/**
	 * \brief Get a resource by name
	 * \return the resource
	 */
	template<typename T, typename... Args>
	T* Get(const std::string& fileName, Args&&... args)
	{
		auto resource = m_ResourcesByName[fileName];

		if (!resource) {
			Load<T>(fileName, std::forward<Args>(args)...);
			resource = m_ResourcesByName[fileName];
		}

		T* res{ static_cast<T*>(resource) };
		if (!res) {
			std::cerr << "The file you asked for does not represent the type you are requesting!" << std::endl;
			return nullptr;
		}

		return res;
	}

	/**
	 * \brief Register a new resource by name
	 * \param resource the resource to register
	 * \param the name of the resource.
	 */
	void RegisterResource(Resource* resource, const std::string& name)
	{
		if (!resource) {
			ERROR_LOG("Resource registration failed: Cannot register a null resource.");
			return;
		}

		if (name.empty()) {
			ERROR_LOG("Resource registration failed: Cannot register a resource without providing a name.");
			return;
		}

		m_ResourcesByName[name] = resource;
		m_ResourcesById[s_Id++] = resource;
	}
};

#endif //RESOURCE_MANAGER_H_
