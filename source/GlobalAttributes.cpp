/* Defines.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "GlobalAttributes.h"

using namespace std;



void GlobalAttributes::Load(const DataNode &node)
{
	if(node.Size() >= 2)
		name = node.Token(1);
	for(const DataNode &child : node)
		if(child.Size() >= 2)
			attributes[child.Token(0)] = child.Value(1);
		else
			child.PrintTrace("Skipping unrecognized attribute:");
}



const string &GlobalAttributes::Name() const
{
	return name;
}



double GlobalAttributes::Get(const string &attribute) const
{
	auto it = attributes.find(attribute);
	return (it == attributes.end()) ? 0. : it->second;
}
