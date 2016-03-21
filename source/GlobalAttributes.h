/* Defines.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef GLOBAL_ATTRIBUTES_H_
#define GLOBAL_ATTRIBUTES_H_

#include <map>
#include <string>

#include "DataNode.h"



// Class storing attributes either for default fallbacks or global engine procedures.
class GlobalAttributes
{
public:
	void Load(const DataNode &node);
	
	const std::string &Name() const;
	
	double Get(const std::string &attribute) const;
	
private:
	std::string name;
	std::map<std::string, double> attributes;
};



#endif
