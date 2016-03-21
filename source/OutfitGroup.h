/* OutfitGroup.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef OUTFIT_GROUP_H_
#define OUTFIT_GROUP_H_

#include "Outfit.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class DataNode;
class Effect;
class Ship;
class Sound;
class Sprite;
class OutfitGroup;



// Class representing a group of outfits that is installed in a ship
// or available in a shop. It encapsulates and manages a map of sets which
// represents not only which outfits are in the group and the age of each
// outfit in the group for purpose of depreciation.
// The Map is keyed by pointer to the outfit in question and the list
// contains the number of outfits of each age, sorted by age, so that
// it is easy to buy lowest price first or sell highest price first.
// Replaces std::map<const Outfit *, int> in many places.
class OutfitGroup
{
private:
	typedef std::map<double, int> InnerMap;
	typedef std::map<const Outfit *, InnerMap> OuterMap;
	
public:
	static int64_t CostFunction(const Outfit *outfit, double wear);
	static double CostFunction(double wear);
	static double UsedWear(const Outfit *outfit);
	static int UsedAge(const Ship *ship);
	static int PlunderAge(const Ship *ship);
	
public:
	OutfitGroup()
	{
		outfits.empty();
	};
	
	void Clear();
	bool Empty() const;
	const std::map<double, int> *Find(const Outfit *outfit) const;
	
	// Given attribute summed over all outfits in group.
	double GetTotalAttribute(std::string attribute) const;
	// Cost of all outfits in group.
	int64_t GetTotalCost() const;
	// Cost of all outfits of a given type.
	int64_t GetTotalCost(const Outfit *outfit) const;
	// How many outfits of a given type.
	int GetTotalCount(const Outfit *outfit) const;
	// Cost of given number of outfits of a given type, either most worn or least worn.
	int64_t GetCost(const Outfit *outfit, int count, bool mostWornFirst) const;
	
	// Add outfits.
	int AddOutfit(const Outfit *outfit, int count, double wear);
	
	// Remove outfits, either most worn first or least worn first. Return number removed.
	int RemoveOutfit(const Outfit *outfit, int count, bool mostWornFirst, OutfitGroup *to = nullptr);
	
	int TransferOutfits(const Outfit *outfit, int count, OutfitGroup *to, bool mostWornFirst, int defaultWear = 0.);
	
	void IncrementDate(int days = 1);
	void SetVariableWear(int meanAge);
	
	// Iterator used to let OutfitGroup be used in for loops just like
	// it was a map<const Outfit *, int>.
	class iterator
	{
	public:
		iterator(const OutfitGroup *group, bool begin);
		iterator(const OutfitGroup *group, const Outfit *outfit);
		
		// Iteration operators
		bool operator!=(const OutfitGroup::iterator &other) const;
		bool operator==(const OutfitGroup::iterator &other) const;
		iterator operator*() const;
		const OutfitGroup::iterator &operator++();
		
		// Getters
		const Outfit *GetOutfit() const;
		double GetWear() const;
		int GetQuantity() const;
		int64_t GetTotalCost() const;
		double GetCostRatio()const;
		std::string GetCostRatioString() const;
		
		
	private:
		const OutfitGroup *myGroup;
		OuterMap::const_iterator outerIter;
		InnerMap::const_iterator innerIter;
		bool isEnd;
	};
	
	
	// Enable use in for loops.
	OutfitGroup::iterator begin() const;
	OutfitGroup::iterator end() const;
	OutfitGroup::iterator find(const Outfit *outfit) const;
	
	
private:
	// Map of outfit to map of ages to outfits of that type of that age.
	OuterMap outfits;
	
};



#endif
