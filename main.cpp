#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Config.hpp>

#include <array>
#include <list>
#include <iostream>
#include <memory>
#include <valarray>
#include <stdexcept>
#include <deque>
#include <map>
#include <random>
#include <cmath>
#include <chrono>

typedef std::pair<float, float> node;
typedef std::list<node> nodeList;

int setSize = 16;
int m_size_x = 512, m_size_y = 512;

// --- --- ---
// im even attempting this project at the first place, because this algorithm worked every time on paper,
// and i just want to test how accurate it really is. It seems to be O(n) time, which suggest it won't work
// perfectly, as this problem is thought to be O(n^2) at best
// --- --- ---

// credit: https://iq.opengenus.org/gift-wrap-jarvis-march-algorithm-convex-hull/
// this is cross product, for determining relative rotation
float getRotation(node &anchorPoint, node &checkedPoint, node &reference)
{
	// r < 0 = counter-clockwise
	// r = 0 = collinear
	// r > 0 = clockwise
	return (checkedPoint.second - anchorPoint.second) * (reference.first - checkedPoint.first) -
		   (checkedPoint.first - anchorPoint.first) * (reference.second - checkedPoint.second);
}

template<typename rngT>
nodeList getRandomSet(int size, int min, int max, rngT &rng) {
	nodeList resultSet;

	for (int i = 0; i < size; i++) {
		resultSet.push_back(
				// note that this is awfully inefficient, just quick to write
					{
						((rng() - rng.min()) / (float) rng.max()) * max + min,
						((rng() - rng.min()) / (float) rng.max()) * max + min
					}
				);
	}

	return resultSet;
}

// credit: https://www.tutorialspoint.com/Jarvis-March-Algorithm
nodeList getConvexHull(nodeList &nodeSet) {
	node ptr = nodeSet.front();

	for (auto &item : nodeSet) {

		// leftmost node
		if (item.first < ptr.first) ptr = item;

	}

	nodeList resultSet;

	do {
		resultSet.push_back(ptr);

		// reference point for getRot..., anything goes as long as it's not ptr
		node rotRef = nodeSet.front();
		if (rotRef == ptr)
			rotRef = nodeSet.back();

		// find the most counter-clockwise point, change rotRef to that point
		for (auto &item : nodeSet) {

			if (getRotation(ptr, item, rotRef) < 0) {
				rotRef = item;
			}

		}

		ptr = rotRef;

	} while (ptr != resultSet.front());

}

nodeList subtractSet(nodeList mainSet, nodeList &unwantedElements) {

	for (auto &item : unwantedElements) {
		mainSet.remove(item);
	}

	return mainSet;
}

float distance(node &a, node &b) {
	// offset to 0,0
	float x = b.first - a.first;
	float y = b.second - a.second;

	// trig vector
	return std::sqrt(x*x + y*y);
}

// beggining with an outer loop, for each iteration find the point that will influence lengthList the least
nodeList calculateDeflate(nodeList &hullSet, nodeList &remainingSet) {
	// 0 -> 1 has index 0
	std::list<float> lengthList; // list as we need to insert values

	// these are outside to simplify loop below
	auto it = hullSet.begin();
	auto last_it = it++;

	while (it != hullSet.end()) {

		lengthList.push_back(distance(*last_it, *it));
		last_it = it;
		it++;

	}

	//manually add the last entry
	lengthList.push_back(distance(hullSet.back(), hullSet.front()));

}

int main() {

	int min = m_size_x < m_size_y ? std::ceil((float)m_size_x * 0.05) : std::ceil((float)m_size_y * 0.05);
	int max = m_size_x < m_size_y ? std::floor((float)m_size_x * 0.95) : std::floor((float)m_size_y * 0.95);

	std::default_random_engine rng(std::chrono::system_clock::now().time_since_epoch().count());

	nodeList initialSet = getRandomSet(setSize, min, max, rng);

	nodeList convexHullSet = getConvexHull(initialSet);
	nodeList remainingSet = subtractSet(initialSet, convexHullSet);

	nodeList finalSet = calculateDeflate(convexHullSet, remainingSet);

	/*
	sf::RenderWindow window(sf::VideoMode(), "gunmo", sf::Style::Fullscreen);
	window.setFramerateLimit(144);

	while(window.isOpen()) {


	}
	 */

	return 0;
}
