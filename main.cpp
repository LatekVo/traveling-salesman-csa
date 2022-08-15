#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <fstream>
#include <string>
#include <array>
#include <list>
#include <iostream>
#include <random>
#include <cmath>
#include <chrono>

typedef std::pair<float, float> node;
typedef std::list<node> nodeList;

unsigned int setSize = 200;
unsigned int m_size_x = 512, m_size_y = 512; // defaults, changed later

// --- --- ---
// im even attempting this project at the first place, because this algorithm worked every time on paper,
// and i just want to test how accurate it really is. It seems to be O(n) time, which suggest it won't work
// perfectly, as this problem is thought to be O(n^2) at best
// --- --- ---
// NOTES:
// convexHull() + deflate() are certanly a great starting points, around 95% of the points allign with the optimal route.
// 95% meaning, this initial set is already in the global-low valley, and just needs a bit more perfecting.
// two-opt / untangle() besides fixing the oblivious (crossing) paths, it also optimizes a lot of other, less oblivious parts of the path.

// TODO: two opt causes two error instances. #1 at .begin() -> .begin()++ iterator, and second at arbitriary points, even though it usually works fine.
// it also breaks indexing of the std::list, causing it to wrongly calculate it's own size.

// credit: https://iq.opengenus.org/gift-wrap-jarvis-march-algorithm-convex-hull/
// this is cross product, for determining relative rotation
float getRotation(node &anchorPoint, node &checkedPoint, node &referenceVector)
{
	// r < 0 = counter-clockwise
	// r = 0 = collinear
	// r > 0 = clockwise
	// highet values at 90deg,
	return (checkedPoint.second - anchorPoint.second) * (referenceVector.first - checkedPoint.first) -
		   (checkedPoint.first - anchorPoint.first) * (referenceVector.second - checkedPoint.second);
}

nodeList loadFromFile(std::string &filename) {
	nodeList resultSet;
	std::ifstream file;
	file.open(filename);
	std::string buffer;
	std::getline(file, buffer);

	


	return resultSet;
}

template<typename rngT>
nodeList getRandomSet(int size, int min, int max, rngT &rng) {
	nodeList resultSet;

	for (int i = 0; i < size; i++) {
		resultSet.push_back(
					{
						std::abs((float) rng()) / rng.max() * (max - min * 2) + min,
						std::abs((float) rng()) / rng.max() * (max - min * 2) + min
					}
				);
	}

	resultSet.unique();

	return resultSet;
}

// reference: https://iq.opengenus.org/gift-wrap-jarvis-march-algorithm-convex-hull/
std::pair<nodeList, nodeList> getConvexHull(nodeList &nodeSet) {

	// TODO: redundant, remove
	// original algo only uses leftmost
	// im using topmost as well as it's a good starting point for the first reference vector
	node leftmost = nodeSet.front();
	node topmost = nodeSet.front();

	std::cout << "probing leftmost point\n";
	for (auto &item : nodeSet) {

		// leftmost node
		if (item.first < leftmost.first) leftmost = item;

		std::cout << item.first << "\n";

	}
	std::cout << "probing topmost point\n";
	for (auto &item : nodeSet) {

		// topmost node
		if (item.second > leftmost.second) topmost = item;

		std::cout << item.second << "\n";

	}

	nodeList hullSet {};
	nodeList interiorSet = nodeSet;

	auto checkedVec = leftmost;
	auto anchorPoint = leftmost;
	auto referenceVec = topmost;

	std::cout << "jarvis loop\n";
	do {
		hullSet.push_back(checkedVec);
		interiorSet.remove(checkedVec);

		anchorPoint = checkedVec; // anchor to last match
		checkedVec = referenceVec; // prepare new vector for matching, such that getRot = 0

		for (auto &randomItem : nodeSet) {
			float rot = getRotation(anchorPoint, randomItem, referenceVec);
			if (rot < 0) {
				referenceVec = randomItem;
			}
		}

		checkedVec = referenceVec; // lock the matching vector, save it as checkedVec
		referenceVec = anchorPoint; // and finally move back the ref to rightmost position, (from next ref's perspective)

	} while (checkedVec != leftmost);

	// intentionally adding overlap, it needs to be added everywhere else anyways
	hullSet.push_back(leftmost);

	// DBG: both sets are set correctly
	return {hullSet, interiorSet};
}

float getDistance(node &a, node &b) {
	// offset to 0,0
	float x = b.first - a.first;
	float y = b.second - a.second;

	// trig vector
	return std::sqrt(x*x + y*y);
}

// beggining with an outer loop, for each iteration find the point that will influence lengthList the least
nodeList calculateDeflate(nodeList hullSet, nodeList remainingSet) {
	std::cout << "starting to deflate\n";
	// 0 -> 1 has index 0
	std::list<float> lengthList; // list as we need to insert values

	auto it = hullSet.begin();
	auto last_it = it;
	it++;

	// generating distances for the lengthList
	do {
		lengthList.push_back(getDistance(*last_it, *it));
		last_it++, it++;
	} while (it != hullSet.end());

	// TODO: implement caching system for getDistance()
	// tip: maybe group floats using std::floor to use std::map as storage?

	unsigned int calcNumber = 1;
	do {
		std::cout << "stage 1, computed #: " << calcNumber << " / " << setSize << "\n";
		calcNumber++;
		auto a_it = hullSet.begin();
		auto b_it = hullSet.begin();
		b_it++;

		// this has an insane time complexity. collapses * edge_points * inner_points * 2;
		// At 50k points... that'll probably run for a couple of hours, or days

		int topDist_pos = 0;
		float topDist_val = std::numeric_limits<float>::max();
		auto topDist_node = remainingSet.front();

		// THIS IS A POINTER TO REMAININGSET, WHY WOULD I USE IT IN HULLSET!???
		// auto topDist_it = remainingSet.begin();

		// for every hull line ...
		int position = 1;
		while (b_it != hullSet.end()) {

			// ... check every unused point
			for (auto &c_it : remainingSet) {

				float check = getDistance(*a_it, c_it) + getDistance(*b_it, c_it) - getDistance(*a_it, *b_it);

				if (check < topDist_val) {
					// std::cout << "new top distance found: " << topDist_val << " -> " << check << "\n";

					topDist_val = check;
					topDist_node = c_it;
					topDist_pos = position;
				}

			}
			a_it++, b_it++, position++;
		}

		// summary: find the smallest distance difference, new hull now includes that point, repeat

		auto insertIter = hullSet.begin();
		for (auto i = 0; i < topDist_pos; i++)
			insertIter++;

		hullSet.insert(insertIter, topDist_node);
 		remainingSet.remove(topDist_node);

	} while (!remainingSet.empty());

	return hullSet;
}

// another n^2 operation
// switch every pair of vectors that's crossing eachother
nodeList untangle(nodeList rawSet) {

	// basic general 2-opt algorithm, working for any two lines, not only the crossed ones

	unsigned int calcNumber = 1;
	// unsigned int cycle = 1;
	// bool isImproved = false;

	calcNumber = 1;

	auto node_a1 = rawSet.begin();
	auto node_a2 = rawSet.begin();
	node_a2++;

	do {
		// std::cout << "stage 2, cycle #: " << cycle << ", computed #: " << calcNumber << " / " << setSize << "\n";
		auto node_b1 = rawSet.begin();
		auto node_b2 = rawSet.begin();
		node_b2++;

		do {

			float dist_a1_a2 = getDistance(*node_a1, *node_a2);
			float dist_b1_b2 = getDistance(*node_b1, *node_b2);

			float dist_a1_b1 = getDistance(*node_a1, *node_b1);
			float dist_a2_b2 = getDistance(*node_a2, *node_b2);

			// DBG ONLY
			// std::cout << "point sets: " << dist_a1_a2 << " " << dist_b1_b2 << " " << dist_a1_b1 << " " << dist_a2_b2 << "\n";

			// for now not bothering with only doing one swap per cycle, one cycle with all swaps sequentially should do just fine.

			if ((dist_a1_b1 + dist_a2_b2) < (dist_a1_a2 + dist_b1_b2)) {

				// the desirable fragment is essentially a sub list. std::list has a reversing option.
				// we should be able to reverse sequence from pointer A to pointer B, in this case, a2 .. b1

				auto b1_next = node_b1;
				b1_next++;
				std::reverse(node_a2, b1_next);

			}

			node_b1++,
			node_b2++;
		} while (node_b2 != rawSet.end());

		node_a1++,
		node_a2++,
		calcNumber++;
	} while (node_a2 != rawSet.end());

	return rawSet;
}

int main() {

	sf::RenderWindow window(sf::VideoMode(), "tsp", sf::Style::Fullscreen);
	m_size_x = window.getSize().x;
	m_size_y = window.getSize().y;

	int min = m_size_x < m_size_y ? std::floor((float)m_size_x * 0.05) : std::floor((float)m_size_y * 0.05);
	int max = m_size_x < m_size_y ? std::ceil((float)m_size_x * 0.95) : std::ceil((float)m_size_y * 0.95);

	std::default_random_engine rng(std::chrono::system_clock::now().time_since_epoch().count());

	nodeList initialSet = getRandomSet(setSize, min, max, rng);

	auto sets = getConvexHull(initialSet);
	nodeList convexHullSet = sets.first;
	nodeList remainingSet = sets.second; // this is fine
	nodeList deflatedSet = calculateDeflate(convexHullSet, remainingSet);
	nodeList untangledSet = untangle(deflatedSet);

	std::cout 	<< "init: " << initialSet.size() << "\n"
				<< "hull: " << convexHullSet.size() << "\n"
				<< "inner: " << remainingSet.size() << "\n"
				<< "deflatedStrip: " << deflatedSet.size() << "\n"
				<< "untangled: " << untangledSet.size() << "\n";

	// DRAWING

	window.setView(window.getDefaultView());
	window.setFramerateLimit(144);
	auto camera = sf::View( sf::FloatRect(0, 0, (float)window.getSize().x, (float)window.getSize().y));
	window.setView(camera);
	window.clear(sf::Color::White);


	sf::CircleShape ptShape;
	ptShape.setRadius(3);
	ptShape.setFillColor(sf::Color::Black);

	std::cout << "drawing points" << std::endl;
	for (auto &point : initialSet) {
		ptShape.setPosition(point.first - ptShape.getRadius(), point.second - ptShape.getRadius());
		window.draw(ptShape);
	}

	sf::VertexArray hullStrip(sf::LineStrip);

	std::cout << "drawing hull line segment" << std::endl;
	for (auto &point : convexHullSet) {
		auto v = sf::Vertex(sf::Vector2f(point.first, point.second));
		v.color = sf::Color::Blue;
		hullStrip.append(v);
	}

	sf::VertexArray deflatedStrip(sf::LineStrip);

	std::cout << "drawing deflatedStrip line segment" << std::endl;
	for (auto &point : deflatedSet) {
		auto v = sf::Vertex(sf::Vector2f(point.first, point.second));
		v.color = sf::Color::Green;
		deflatedStrip.append(v);
	}

	sf::VertexArray untangledStrip(sf::LineStrip);

	std::cout << "drawing untangled line segment" << std::endl;
	for (auto &point : untangledSet) {
		auto v = sf::Vertex(sf::Vector2f(point.first, point.second));
		v.color = sf::Color::Red;
		untangledStrip.append(v);
	}

	window.draw(hullStrip);
	window.draw(deflatedStrip);
	window.draw(untangledStrip);

	window.display();

	sf::Clock g_delta;
	float g_currentFrameAdjustment = 0.f;
	sf::Vector2f position {0, 0};

	while(window.isOpen()) {
		//window.clear(sf::Color::White);

		// this has to be checked more often, in case exec time gets insane
		sf::Event event {};
		while(window.pollEvent(event)) {
			switch (event.type) {
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::KeyPressed:
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
						window.close();
					}
					break;
			}
		}

		float mov = 2 * g_currentFrameAdjustment * 1000;
		float xMov = 0;
		float yMov = 0;

		g_currentFrameAdjustment = g_delta.getElapsedTime().asSeconds();
		g_delta.restart();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
			yMov -= mov;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
			yMov += mov;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			xMov -= mov;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			xMov += mov;

		if (xMov != 0 && yMov != 0) {
			// 707 = sin 45deg = cos 45deg
			xMov *= .707;
			yMov *= .707;
		}

		position.x += xMov;
		position.y += yMov;

		camera.setCenter(position);

		window.display();

	}

	return 0;
}
