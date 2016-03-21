/* OutfitGroup.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "OutfitGroup.h"

#include "Outfit.h"
#include "DataNode.h"
#include "Format.h"
#include "GameData.h"
#include "Random.h"
#include "Ship.h"

#include <cmath>
#include <vector>

using namespace std;



int64_t OutfitGroup::CostFunction(const Outfit *outfit, double wear)
{
	return static_cast<int64_t>(outfit->Cost() * (1. - wear));
}



double OutfitGroup::CostFunction(double wear)
{
	return (1. - wear);
}



double OutfitGroup::UsedWear(const Outfit *outfit)
{
	// Random between 30% and 90% depreciated.
	if(outfit->Get("ageless") || outfit->Category() == "Ammunition")
		return 0.;
	int minAge = outfit->Get("min used age");
	int maxAge = outfit->Get("max used age");
	int age = minAge + Random::Int(maxAge - minAge);
	return min(outfit->Get("max wear"), age * outfit->Get("wear per day"));
}



int OutfitGroup::UsedAge(const Ship *ship)
{
	// Random between 30% and 90% depreciated.
	int minAge = ship->BaseAttributes().Get("min used age");
	int maxAge = ship->BaseAttributes().Get("max used age");
	return (minAge + Random::Int(maxAge - minAge));
}



int OutfitGroup::PlunderAge(const Ship *ship)
{
	// Random between 90% and 100% depreciated.
	double minWear = ship->BaseAttributes().Get("min plunder wear");
	double maxWear = ship->BaseAttributes().Get("max plunder wear");
	return (minWear + Random::Int(maxWear - minWear));
}



void OutfitGroup::Clear()
{
	outfits.clear();
}



bool OutfitGroup::Empty() const
{
	return outfits.empty();
}



const map<double, int> *OutfitGroup::Find(const Outfit *outfit) const
{
	auto iter = outfits.find(outfit);
	if(iter != outfits.end())
		return &iter->second;
	return nullptr;
}



double OutfitGroup::GetTotalAttribute(string attribute) const
{
	// Get the total value of an attribute (such as mass) over every outfit.
	double value = 0;
	for(auto it = begin(); it != end(); ++it)
		value += it.GetOutfit()->Get(attribute) * it.GetQuantity();
	return value;
}



int64_t OutfitGroup::GetTotalCost() const
{
	// Get the total cost of every outfit.
	int64_t cost = 0;
	for(auto it = begin(); it != end(); ++it)
		cost += it.GetTotalCost();
	return cost;
}



int64_t OutfitGroup::GetTotalCost(const Outfit *outfit) const
{
	int64_t cost = 0;
	for(auto it = find(outfit); it.GetOutfit() == outfit; ++it)
		cost += it.GetTotalCost();
	return cost;
}



int OutfitGroup::GetTotalCount(const Outfit *outfit) const
{
	int count = 0;
	auto matchingOutfits = outfits.find(outfit);
	if(matchingOutfits == outfits.end())
		return 0;
	for(auto it : matchingOutfits->second)
		count += it.second;
	return count;
}



int64_t OutfitGroup::GetCost(const Outfit *outfit, int count,
							 bool mostWornFirst) const
{
	int64_t cost = 0;
	auto matchingOutfits = outfits.find(outfit);
	if(matchingOutfits == outfits.end())
		return 0; // Don't have any.
	if(mostWornFirst)
	{
		auto it = matchingOutfits->second.rbegin();
		for(; it != matchingOutfits->second.rend(); ++it)
		{
			int matched = min(it->second, count);
			cost += CostFunction(outfit, it->first) * matched;
			count -= matched;
			if(count <= 0)
				break;
		}
	}
	else
	{
		// newest first
		auto it = matchingOutfits->second.begin();
		for(; it != matchingOutfits->second.end(); ++it)
		{
			int matched = min(it->second, count);
			cost += CostFunction(outfit, it->first) * matched;
			count -= matched;
			if(count <= 0)
				break;
		}
	}
	return cost;
}



// Can be used to remove outfits, but will only remove outfits of the specified wear.
int OutfitGroup::AddOutfit(const Outfit *outfit, int count, double wear)
{
	auto oit = outfits.find(outfit);
	if(oit == outfits.end())
	{
		outfits[outfit] = InnerMap();
		outfits[outfit][wear] = count;
	}
	else
	{
		auto iit = oit->second.find(wear);
		if(iit == oit->second.end())
			oit->second[wear] = count;
		else
		{
			oit->second[wear] += count;
			if(!oit->second[wear])
				oit->second.erase(iit);
		}
		if(oit->second.empty())
			outfits.erase(oit);
	}
	return count;
}



// Remove outfits of a given type, either most worn or least worn first.
// Used for making transfers as well.
int OutfitGroup::RemoveOutfit(const Outfit *outfit, int count, bool mostWornFirst, OutfitGroup *to)
{
	auto oit = outfits.find(outfit);
	if(oit == outfits.end())
		return 0;
		
	int removed = 0;
	if(mostWornFirst)
	{
		auto iit = oit->second.end();
		vector<InnerMap::iterator> toErase;
		do
		{
			--iit;
			double wear = iit->first;
			int toRemove = min(iit->second, count - removed);
			removed += toRemove;
			oit->second[wear] -= toRemove;
			if(to)
				to->AddOutfit(outfit, toRemove, wear);
			if(!oit->second[wear])
			{
				toErase.push_back(iit);
			}
		}
		while(count > removed && iit != oit->second.begin());
		
		for(auto it : toErase)
			oit->second.erase(it);
	}
	else
	{
		auto iit = oit->second.begin();
		while(count > removed && iit != oit->second.end())
		{
			double wear = iit->first;
			int toRemove = min(iit->second, count - removed);
			removed += toRemove;
			oit->second[wear] -= toRemove;
			if(to)
				to->AddOutfit(outfit, toRemove, wear);
			if(!oit->second[wear])
				oit->second.erase(iit++); // erase current and iterate.
			else
				++iit; // just iterate.
		}
	}
	if(oit->second.empty())
		outfits.erase(oit);
		
	return removed;
}



// Needs to support all kinds of operations either on a group or between groups.
int OutfitGroup::TransferOutfits(const Outfit *outfit, int count, OutfitGroup *to, bool mostWornFirst, int defaultWear)
{
	// Invalid inputs.
	if(!count || !outfit)
		return 0;
	// Use add/remove if there's no destination.
	if(!to)
	{
		if(count > 0)
			return RemoveOutfit(outfit, count,
								mostWornFirst); // Transfer to nowhere = remove.
		else
			return -AddOutfit(outfit, -count, defaultWear); // Transfer from nowhere = add.
	}
	// If count is negative but *to is valid, just turn the whole thing around.
	if(count < 0)
		return -(to->TransferOutfits(outfit, -count, this, mostWornFirst, defaultWear));
	// Transferring a positive number of outfits to a valid destination.
	// Use the remove function for this.
	return RemoveOutfit(outfit, count, mostWornFirst, to);
}



// Go through the whole group and increment all the wears.
void OutfitGroup::IncrementDate(int days)
{
	for(auto oit : outfits)
	{
		if(oit.first->Get("ageless") || oit.first->Category() == "Ammunition")
			continue;
		// make a copy
		InnerMap temp;
		temp.insert(oit.second.begin(), oit.second.end());
		// clear
		outfits[oit.first].clear();
		// re-insert with incremented wears
		for(auto iit : temp)
			outfits[oit.first][min(oit.first->Get("max wear"), iit.first + days * oit.first->Get("wear per day"))] = iit.second;
	}
}



// Set this group of outfits to a range of ages, simulating those on a used ship.
void OutfitGroup::SetVariableWear(int meanAge)
{
	for(auto oit : outfits)
	{
		if(oit.first->Get("ageless") || oit.first->Category() == "Ammunition")
			continue;
		int count = GetTotalCount(oit.first);
		// clear
		outfits[oit.first].clear();
		// re-build
		double ageStdDev = oit.first->Get("age standard deviation");
		// 0% of new outfits are replaced, 50% of outfits are replaced at half-life, 100% of outfits are replaced at infinity
		double variantChance = meanAge / (meanAge + oit.first->Get("half-life"));
		for(int i = 0; i < count; ++i)
		{
			double age = meanAge;
			if(Random::Real() < variantChance)
			{
				age += Random::Normal() * ageStdDev;
			}
			++outfits[oit.first][min(oit.first->Get("max wear"), age * oit.first->Get("wear per day"))];
		}
	}
}



OutfitGroup::iterator::iterator(const OutfitGroup *group, bool begin)
{
	myGroup = group;
	if(begin && !myGroup->outfits.empty())
	{
		outerIter = myGroup->outfits.begin();
		innerIter = outerIter->second.begin();
		isEnd = false;
	}
	else
	{
		outerIter = myGroup->outfits.end();
		innerIter = outerIter->second.begin();
		isEnd = true;
	}
}



OutfitGroup::iterator::iterator(const OutfitGroup *group, const Outfit *outfit)
{
	myGroup = group;
	isEnd = false;
	outerIter = myGroup->outfits.find(outfit);
	if(outerIter == myGroup->outfits.end())
		isEnd = true;
	innerIter = outerIter->second.begin();
	if(innerIter == outerIter->second.end())
		isEnd = true;
		
}



// Operators that form allow use with a range-based for loop
bool OutfitGroup::iterator::operator!=(const OutfitGroup::iterator &other) const
{
	if(isEnd && other.isEnd)
		return false;
	return myGroup != other.myGroup || outerIter != other.outerIter || innerIter != other.innerIter;
}



bool OutfitGroup::iterator::operator==(const OutfitGroup::iterator &other) const
{
	return outerIter == other.outerIter && innerIter == other.innerIter;
}



OutfitGroup::iterator OutfitGroup::iterator::operator*() const
{
	return *this;
}



const OutfitGroup::iterator &OutfitGroup::iterator::operator++()
{
	if(++innerIter == outerIter->second.end())
	{
		if(++outerIter == myGroup->outfits.end())
			isEnd = true;
		else
			innerIter = outerIter->second.begin();
	}
	return *this;
}



const Outfit *OutfitGroup::iterator::GetOutfit() const
{
	return outerIter->first;
}



double OutfitGroup::iterator::GetWear() const
{
	return innerIter->first;
}



int OutfitGroup::iterator::GetQuantity() const
{
	return innerIter->second;
}



int64_t OutfitGroup::iterator::GetTotalCost() const
{
	return OutfitGroup::CostFunction(GetOutfit(), GetWear()) * GetQuantity();
}



double OutfitGroup::iterator::GetCostRatio() const
{
	return CostFunction(GetWear());
}



string OutfitGroup::iterator::GetCostRatioString() const
{
	int64_t maxCost = myGroup->GetCost(GetOutfit(), 1, false);
	int64_t minCost = myGroup->GetCost(GetOutfit(), 1, true);
	int64_t baseCost = GetOutfit()->Cost();
	if(minCost == maxCost)
		return Format::Percent(minCost, baseCost);
	return Format::Percent(minCost, baseCost) + "-" + Format::Percent(maxCost, baseCost);
}


// OutfitGroup functions that return iterators.
OutfitGroup::iterator OutfitGroup::begin() const
{
	return iterator(this, true);
}



OutfitGroup::iterator OutfitGroup::end() const
{
	return iterator(this, false);
}



OutfitGroup::iterator OutfitGroup::find(const Outfit *outfit) const
{
	return iterator(this, outfit);
}
