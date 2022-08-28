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
#include <sstream>

typedef std::pair<float, float> node;
typedef std::list<node> nodeList;

unsigned int setSize = 400;
unsigned int m_size_x = 512, m_size_y = 512; // defaults, changed later
float optimalLength = 0;

// --- --- ---
// the reason im even attempting this project at the first place, is because this algorithm worked every time on paper,
// and i just want to test how accurate it really is. It seems to be O(n) time, which suggest it won't work
// perfectly, as this problem is thought to be O(n^2) at best
// UPDATE AFTER FINISHING ^^^
// while
// --- --- ---
// NOTES:
// convexHull() + deflate() are certainly a great starting points, around 95% of the points allign with the optimal route.
// 95% meaning, this initial set is already in the global-low valley, and just needs a bit more perfecting.
// two-opt / untangle() besides fixing the oblivious (crossing) paths, it also optimizes a lot of other, less oblivious parts of the path.
// --- --- ---
// POTENTIAL GOALS:
// biggest pain right now is not being able to untangle() while changing up (rotating) 3 connections instead of 2.
// while it's easily programmable, complexity would jump to n^3 which is way too much - much more than I could afford.
// currently, how this could be done, is by iterating by [every node pair] -> [every node] -> [every node pair].

// credit: https://iq.opengenus.org/gift-wrap-jarvis-march-algorithm-convex-hull/
// this is cross product, for determining relative rotation
float getRotation(node &anchorPoint, node &checkedPoint, node &referenceVector)
{
	// r < 0 = counter-clockwise
	// r = 0 = collinear
	// r > 0 = clockwise
	// highest values at 90deg,
	return (checkedPoint.second - anchorPoint.second) * (referenceVector.first - checkedPoint.first) -
		   (checkedPoint.first - anchorPoint.first) * (referenceVector.second - checkedPoint.second);
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

float getDistance(node &a, node &b) {
	// offset to 0,0
	float x = b.first - a.first;
	float y = b.second - a.second;

	// trig vector
	return std::sqrt(x*x + y*y);
}

float sumPath(nodeList &nodePath) {
	auto node_a = nodePath.begin();
	auto node_b = nodePath.begin();
	node_b++;

	float pathTotal = 0;

	while (node_b != nodePath.end()) {

		pathTotal += getDistance(*node_a, *node_b);

		node_a++,
		node_b++;
	}

	return pathTotal;
}

union int_or_float {
	int integer;
	float decimal;
};

// TODO: probably should rewrite this function to contain all of the keywords in some sort of a string - function table.
nodeList loadFromFile(char *filename) {

	std::cout << "running file loading on file: " << filename << "\n";

	nodeList resultSet;
	std::ifstream file;

	file.open(filename);
	std::cout << (file.good() ? "file found" : "file not found") << '\n';

	std::string buffer;

	unsigned int totalSize = 0;
	std::string commentBuffer = "COMMENTS:\n";

	while(std::getline(file, buffer)) {
		std::cout << "dbg\n";
		std::stringstream bufferSStream(buffer);
		std::string option, value;

		bufferSStream >> option;

		// --- sorting section ---

		// parsing through all of the coords
		if (option == "NODE_COORD_SECTION") {
			buffer.clear();
			while(std::getline(file, buffer)) {

				// there is some rubbish attached to "EOF" that i prefer to get rid of this way
				std::stringstream quickfix;
				std::string str_quickfix;

				quickfix.str(buffer);
				quickfix >> str_quickfix;

				if (str_quickfix == "EOF")
					return resultSet;
				std::cout << "still here\n";

				std::string index, x_pointCoord, y_pointCoord;

				bufferSStream.clear();
				bufferSStream.str(buffer);
				bufferSStream >> index >> x_pointCoord >> y_pointCoord;
				buffer.clear();

				auto toFloat = [](std::string &s) {
					if (s.find('.'))
						return std::stof(s);
					else
						return (float) std::stoi(s);
				};

				// todo: read DIMENSIONS field
				totalSize = std::stoi(index);

				resultSet.push_back({toFloat(x_pointCoord), toFloat(y_pointCoord)});

				std::cout << "loading... " << index << '/' << totalSize << " '" << x_pointCoord << " " << y_pointCoord << "'\n";
			}
		} else {
			bufferSStream >> buffer;
			value = bufferSStream.str();

			if (option == "COMMENT") {
				commentBuffer += value + '\n';
			}
			/*
			if (option == "DIMENSION") {
				totalSize = std::stoi(value);
			}
			*/
		}

		std::cout << "buffer: " << buffer << " option: " << option << " value: " << value << '\n';

		buffer.clear();
		bufferSStream.clear();
	}

	totalSize = resultSet.size();

	std::cout << commentBuffer
			  << "SIZE: " << totalSize << '\n';

	setSize = totalSize;

	// don't want to risk errors, and i saw some duplicated points in these datasets.
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

	}
	std::cout << "probing topmost point\n";
	for (auto &item : nodeSet) {

		// topmost node
		if (item.second > topmost.second && item != leftmost) topmost = item;

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

	while (!remainingSet.empty()) {

		int topDist_pos = 0;
		float topDist_val = std::numeric_limits<float>::max();
		auto topDist_node = remainingSet.front();

		auto a_it = hullSet.begin();
		auto b_it = a_it;
		b_it++;

		// this has an insane time complexity. collapses * edge_points * inner_points * 2;
		// At 50k points... that'll probably run for a couple of hours, or days

		// for every hull line ...
		int position = 1;
		while (b_it != hullSet.end()) {

			// ... check every unused point
			for (auto &c_it: remainingSet) {

				float check = getDistance(*a_it, c_it) + getDistance(*b_it, c_it) - getDistance(*a_it, *b_it);

				if (check < topDist_val) {
					// std::cout << "new top distance found: " << topDist_val << " -> " << check << "\n";

					topDist_val = check;
					topDist_node = c_it;
					topDist_pos = position;
				}

				// PREVIOUS INSERTION POSITION.

			}
			a_it++, b_it++, position++;
		}

		// moved inserting here, this will increase the execution time, but will increase the accuracy.
		// after finding the smallest distance difference, apply it.
		auto insertIter = hullSet.begin();
		for (auto i = 0; i < topDist_pos; i++)
			insertIter++;

		hullSet.insert(insertIter, topDist_node);
		remainingSet.remove(topDist_node);

	}

	return hullSet;

}

// another n^2 operation
// switch every pair of vectors that's crossing eachother
nodeList untangle(nodeList rawSet) {
	std::cout << "starting to untangle\n";

	// basic general 2-opt algorithm, working for any two lines, not only the crossed ones

	unsigned int calcNumber = 1;
	bool repeat = true;

	while (repeat) {
		repeat = false;

		calcNumber = 1;

		auto node_a1 = rawSet.begin();
		auto node_a2 = node_a1;
		node_a2++;

		// debug only
		int iterCounter_a = 0, iterCounter_b = 0;

		iterCounter_a = 0;

		while (node_a2 != rawSet.end()) {
			// std::cout << "stage 2, cycle #: " << cycle << ", computed #: " << calcNumber << " / " << setSize << "\n";
			auto node_b1 = node_a1;
			node_b1++;
			auto node_b2 = node_b1;
			node_b2++;

			iterCounter_b = 0;

			while (node_b2 != rawSet.end()) {

				// float dist_a1_a2, dist_b1_b2, dist_a1_b1, dist_a2_b2;
				float distPair_a, distPair_b;

				// multiplying sub-distances instead of getting the total distance actually provides better results lol
				// there is an error where around half of inefficiencies are not caught, even though they should be

				distPair_a = getDistance(*node_a1, *node_a2) + getDistance(*node_b1, *node_b2);
				distPair_b = getDistance(*node_a1, *node_b1) + getDistance(*node_a2, *node_b2);
				std::cout << distPair_a << " " << distPair_b << "\n";
				// DBG ONLY
				// std::cout << "point sets: " << dist_a1_a2 << " " << dist_b1_b2 << " " << dist_a1_b1 << " " << dist_a2_b2 << "\n";

				// for now not bothering with only doing one swap per cycle, one cycle with all swaps sequentially should do just fine.

				// quickfix: iter != iter, shouldn't have to do this
				if (distPair_b < distPair_a) {

					// the desirable fragment is essentially a sub list. std::list has a reversing option.
					// we should be able to reverse sequence from pointer A to pointer B, in this case, a2 .. b1

					auto b1_next = node_b1;
					b1_next++;
					std::reverse(node_a2, b1_next);

					std::cout << rawSet.size() << " fix @ " << iterCounter_a << " " << iterCounter_b << "\n";

					repeat = true;
				}

				untangleSkip:

				node_b1++,
				node_b2++,
				iterCounter_b++;
			}


			node_a1++,
			node_a2++,
			iterCounter_a++,
			calcNumber++;
		}

	}
	return rawSet;
}

int main(int argc, char* argv[]) {

	bool usingFile = false;
	sf::RenderWindow window(sf::VideoMode(), "tsp", sf::Style::Fullscreen);
	m_size_x = window.getSize().x;
	m_size_y = window.getSize().y;

	auto getInitSet = [&](){
		if (argc == 1) {

			int min = m_size_x < m_size_y ? std::floor((float) m_size_x * 0.05) : std::floor((float) m_size_y * 0.05);
			int max = m_size_x < m_size_y ? std::ceil((float) m_size_x * 0.95) : std::ceil((float) m_size_y * 0.95);

			std::default_random_engine rng(std::chrono::system_clock::now().time_since_epoch().count());

			return getRandomSet(setSize, min, max, rng);

		} else {

			usingFile = true;
			return loadFromFile(argv[1]);

		}
	};
	// IMPORTANT NOTE: untangle() stopped working, deflate was causing errors i was conviced were caused by untangledSet
	// ^^^ initially fixed the deflate() errors (probably), untangle() getting stuck persists

	nodeList initialSet = getInitSet();

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
	ptShape.setFillColor(sf::Color::Black);

	sf::VertexArray hullStrip(sf::LineStrip);
	sf::VertexArray deflatedStrip(sf::LineStrip);
	sf::VertexArray untangledStrip(sf::LineStrip);

	std::cout << "drawing hull line segment" << std::endl;
	for (auto &point : convexHullSet) {
		auto v = sf::Vertex(sf::Vector2f(point.first, point.second));
		v.color = sf::Color::Blue;
		hullStrip.append(v);
	}


	std::cout << "drawing deflatedStrip line segment" << std::endl;
	for (auto &point : deflatedSet) {
		auto v = sf::Vertex(sf::Vector2f(point.first, point.second));
		v.color = sf::Color::Green;
		deflatedStrip.append(v);
	}


	std::cout << "drawing untangled line segment" << std::endl;
	for (auto &point : untangledSet) {
		auto v = sf::Vertex(sf::Vector2f(point.first, point.second));
		v.color = sf::Color::Red;
		untangledStrip.append(v);

	}

	std::cout << "deflate path length: " << sumPath(deflatedSet) << "\n";
	std::cout << "untangle path length: " << sumPath(untangledSet) << "\n";
	std::cout << "optimal path length: " << (optimalLength == 0 ? "N/A" : std::to_string(optimalLength)) << "\n";

	sf::Clock g_delta;
	float g_currentFrameAdjustment = 0.f;
	sf::Vector2f position {0, 0};
	float currentZoom = 1.f;

	while(window.isOpen()) {
		float zoomChange = 1.f;

		g_currentFrameAdjustment = g_delta.getElapsedTime().asSeconds();
		g_delta.restart();

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
				case sf::Event::MouseWheelScrolled:
					if (event.mouseWheelScroll.delta != 0)
						zoomChange -= event.mouseWheelScroll.delta * 0.10;
			}
		}

		ptShape.setRadius(3 * currentZoom);
		for (auto &point : initialSet) {
			ptShape.setPosition(point.first - ptShape.getRadius(), point.second - ptShape.getRadius());
			window.draw(ptShape);
		}

		window.draw(hullStrip);
		window.draw(deflatedStrip);
		window.draw(untangledStrip);

		float mov = 2 * g_currentFrameAdjustment * 1000 * currentZoom;
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

		camera.zoom(zoomChange);
		currentZoom *= zoomChange;
		zoomChange = 1;

		camera.setCenter(position);
		if(usingFile) {
			window.setView(camera);
		}
		window.display();

	}

	return 0;
}
