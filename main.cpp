#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <array>
#include <list>
#include <iostream>
#include <random>
#include <cmath>
#include <chrono>

typedef std::pair<float, float> node;
typedef std::list<node> nodeList;

unsigned int setSize = 16;
unsigned int m_size_x = 512, m_size_y = 512;

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
std::pair<nodeList, nodeList> getConvexHull(nodeList &nodeSet) {
	// I had issues with the original code and raw pointers somehow work, so I will go with that
	node *ptr = &(nodeSet.front());

	// DBG: THIS ONE WORKS
	for (auto &item : nodeSet) {

		// leftmost node
		if (item.first < ptr->first) ptr = &item;

	}

	nodeList hullSet {};
	nodeList interiorSet = nodeSet;

	do {
		hullSet.push_back(*ptr);
		interiorSet.remove(*ptr);

		// reference point for getRot..., anything goes as long as it's not ptr
		node rotRef = nodeSet.front();
		if (rotRef == *ptr)
			rotRef = nodeSet.back();

		// find the most counter-clockwise point, change rotRef to that point
		for (auto &item : nodeSet) {

			if (getRotation(*ptr, item, rotRef) < 0) {
				rotRef = item;
			}

		}

		ptr = &rotRef;

	} while (*ptr != hullSet.front());

	return {hullSet, nodeSet};
}

float getDistance(node &a, node &b) {
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
	{ // inner localization
		auto it = hullSet.begin();
		auto last_it = it++;

		while (it != hullSet.end()) {

			lengthList.push_back(getDistance(*last_it, *it));
			last_it = it;
			it++;

		}
	}

	//manually add the last entry
	lengthList.push_back(getDistance(hullSet.back(), hullSet.front()));

	// TODO: implement caching system for getDistance()
	// tip: group floats for std::map using std::floor()

	do {
		auto a_it = hullSet.begin();
		auto b_it = hullSet.begin()++;

		float topDistance = std::numeric_limits<float>::max();
		auto topDist_iterator = remainingSet.begin();

		// this has an insane time complexity. no_collapses * no_edge_points * no_inner_points * 2;
		// At 50k points...

		for (; a_it != hullSet.end(); a_it++, b_it++) {

			if (b_it == hullSet.end())
				b_it = hullSet.begin();


			for (auto it = remainingSet.begin(); it != remainingSet.end(); it++) {
				float check = getDistance(*a_it, *it) + getDistance(*a_it, *it);
				if (check < topDistance) {
					topDistance = check;
					topDist_iterator = it;
				}
			}

			//for (auto &innerNode : remainingSet) {}


			// TODO: also cache at least one potential closest-distance node here, most of them will never change over time,
			// and even if they do, only a couple of other known values need to be checked, compared to the whole remainingSet
		}

		// for lengthList(): remove a -> b connection, establish a -> n -> b
		// let's say a.i = 5, then a -> n = 5i, n -> b = 6i, n.i = 6, everything beyond n is shifted.
		// inserting right before the element iterator is pointing to, and it's format is: insert(iterator, value)
		hullSet.insert(topDist_iterator, *topDist_iterator);
		remainingSet.remove(*topDist_iterator);

	} while (!remainingSet.empty());

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
	nodeList remainingSet = sets.second;

	nodeList finalSet = calculateDeflate(convexHullSet, remainingSet);

	std::cout 	<< initialSet.size() << "\n"
				<< convexHullSet.size() << "\n"
				<< remainingSet.size() << "\n" // ERR
				<< finalSet.size() << "\n"; // ERR

	// DRAWING

	window.setView(window.getDefaultView());
	window.setFramerateLimit(144);
	auto camera = sf::View( sf::FloatRect(0, 0, (float)window.getSize().x, (float)window.getSize().y));
	window.setView(camera);
	window.clear(sf::Color::White);


	sf::CircleShape ptShape;
	ptShape.setRadius(15);
	ptShape.setFillColor(sf::Color::Black);

	for (auto &point : remainingSet) {
		ptShape.setPosition(point.first, point.second);
		window.draw(ptShape);
		std::cout << "drawing point\n";
	}

	sf::VertexArray hullStrip(sf::LineStrip);

	for (auto &point : convexHullSet) {
		hullStrip.append(sf::Vertex(sf::Vector2f(point.first, point.second)));
	}

	window.draw(hullStrip);

	window.display();

	sf::Clock g_delta;
	float g_currentFrameAdjustment = 0.f;
	sf::Vector2f position {0, 0};

	while(window.isOpen()) {
		window.clear(sf::Color::White);

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

		window.display();

	}


	return 0;
}
