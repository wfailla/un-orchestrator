#ifndef NATIVE_CAPABILITY_H_
#define NATIVE_CAPABILITY_H_

#include <string>

enum captype_t {
	EXECUTABLE,
	SCRIPT
};

class Capability {
private:

	/*
	 * @brief: name of the capability
	 */
	std::string name;

	/*
	 * @brief: path at which the capability is located (./ if in $PATH)
	 */
	std::string path;

	/*
	 * @brief: type of the capability
	 */
	captype_t type;

public:

	Capability(std::string name, std::string path, captype_t type);
	//it cloud be derived
	virtual ~Capability();

	/*
	 * @brief: returns the name of the capability
	 */
	std::string getName();

	/*
	 * @brief: returns the path of the capability
	 */
	std::string getPath();

	/*
	 * @brief: returns the type of the capability
	 */
	captype_t getType();

};

#endif /* NATIVE_CAPABILITY_H_ */
