#ifndef RESOURCE_H_
#define RESOURCE_H_

#include <string>
#include "types.h"

/**
 * \brief Resource class of the engine
 * \details A resource is type agnostic.
 */
class Resource {
private:
	/**
	 * \brief ID of the resource
	 */
	ui32 m_Id;

public:
	Resource() : m_Id{ 0 }
	{
	}

	explicit Resource(ui32 id) : m_Id{ id }
	{
	}

	virtual ~Resource() = default;

	/**
	 * \brief Getter of the resource id
	 * \return Resource ID
	 */
	ui32 GetId() const noexcept
	{
		return m_Id;
	}

	/**
	 * \brief Setter for the resource ID
	 * \param id the resource ID
	 */
	void SetId(ui32 id) noexcept
	{
		m_Id = id;
	}

	/**
	 * \brief Load a resource form a file
	 * \param file_name The path of the file where the resource is stored.
	 * \return TRUE if the loading has been successful, false otherwise.
	 */
	virtual bool Load(const std::string& fileName) noexcept = 0;
};

#endif //RESOURCE_H_
