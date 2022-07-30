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

typedef std::pair<float, float> node;
typedef std::list<node> nodeList;

int setSize = 20;
int m_size_x = 500, m_size_y = 500;

// --- --- ---
// im even attempting this project at the first place, because this algorithm worked every time on paper,
// and i just want to test how accurate it really is. It seems to be O(n) time, which suggest it won't work
// perfectly, as this problem is thought to be O(n^2) at best
// --- --- ---

nodeList getRandomSet(int size) {

}

nodeList getConvexHull(nodeList &nodeSet) {
	node ptr = nodeSet.front();

	for (auto &item : nodeSet) {

		// leftmost node
		if (item.first < ptr.first) ptr = item;

	}

	nodeList resultSet = {ptr};



}

nodeList subtractSet(nodeList &mainSet, nodeList &unwantedSet) {

}

int main() {

	nodeList initialSet = getRandomSet(setSize);

	nodeList convexHullSet = getConvexHull(initialSet);
	nodeList remainingSet = subtractSet(initialSet, convexHullSet);

	nodeList finalSet;

	sf::RenderWindow window(sf::VideoMode(), "gunmo", sf::Style::Fullscreen);
	window.setFramerateLimit(144);

	while(window.isOpen()) {


	}

	return 0;
}
