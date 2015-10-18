#ifndef NATIVE_CAPABILITY_H_
#define NATIVE_CAPABILITY_H_

#include <string>

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

public:

	Capability(std::string name, std::string path);
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

};

#endif /* NATIVE_CAPABILITY_H_ */
