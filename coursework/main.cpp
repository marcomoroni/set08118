#include <SFML\Graphics.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>

#define WINDOW_HEIGHT 700
#define WINDOW_WIDTH 1000

using namespace std;
using namespace sf;

class Node
{
public:
	Node() = default;
	string name;
	CircleShape shape;
	Vector2i pos;
	vector<Node*> connections = {};
};

class AI
{
public:
	AI() = default;
	vector<Node*> openSet;
	vector<Node*> closedSet;
	Node* currentNode;
	void nextStep();
	bool isFinished = false;
};

AI ai;

int noOfCaverns;
vector<Node*> caverns;
Node* startCavern;
Node* endCavern;

vector<CircleShape> dots; // Grid

float biggestXCoord = 0.f;
float biggestYCoord = 0.f;
float margin = 100.f;

bool autoMode = false; // is the user going to do step by step?

float calculate_h(Node* current, Node* end)
{
	return sqrt((end->pos.x - current->pos.x) * (end->pos.x - current->pos.x) + (end->pos.y - current->pos.y) * (end->pos.y - current->pos.y));
}

float calculate_g(float currentG, Node* current, Node* connection)
{
	return currentG + sqrt((connection->pos.x - current->pos.x) * (connection->pos.x - current->pos.x) + (connection->pos.y - current->pos.y) * (connection->pos.y - current->pos.y));
}

// A*
void AI::nextStep()
{
	cout << "Stepping..." << endl;
	/*if (!openSet.empty())
	{
		for (auto connection : currentNode->connections)
		{
			// if connected node is not in closed set
			if (!(find(closedSet.begin(), closedSet.end(), connection) != closedSet.end()))
			{
				openSet.push_back(connection);
			}
		}
		for (auto cavern : openSet)
		{
			// Calculate evaluation function
			//startCavern->setG(0);
			//startCavern->calcH();
		}
	}*/
}

void Reset()
{

}

void Load()
{
	string path = "res/input1.cav";

	string buffer;

	// Load in file to buffer
	ifstream f(path);
	if (f.good())
	{
		f.seekg(0, ios::end);
		buffer.resize(f.tellg());
		f.seekg(0);
		f.read(&buffer[0], buffer.size());
		f.close();
	}
	else
	{
		throw string("Couldn't open cavern file: ") + path;
	}

	// DEBUG
	cout << "Buffer: " << buffer << endl;
	cout << endl;

	// Get numbers
	stringstream ss(buffer);
	vector<int> data;
	while (ss.good())
	{
		string substr;
		getline(ss, substr, ',');
		data.push_back(stoi(substr));
	}

	// Number of caves
	noOfCaverns = data.at(0);
	data.erase(data.begin());

	// DEBUG
	cout << "noOfCaverns: " << to_string(noOfCaverns) << endl;
	cout << endl;

	// Coordinates and name
	for (int i = 0; i < noOfCaverns; i++)
	{
		auto newCavern = new Node();
		newCavern->pos = { data.at(0), data.at(1) };
		newCavern->name = to_string(i + 1);
		data.erase(data.begin());
		data.erase(data.begin());
		caverns.push_back(newCavern);

		// First and last caverns
		if (i == 0) {
			startCavern = newCavern;
		}
		if (i == noOfCaverns - 1)
		{
			endCavern = newCavern;
		}

		// Set biggest coords
		if (newCavern->pos.x > biggestXCoord)
		{
			biggestXCoord = newCavern->pos.x;
		}
		if (newCavern->pos.y > biggestYCoord)
		{
			biggestYCoord = newCavern->pos.y;
		}
	}

	// DEBUG
	for (int i = 0; i < noOfCaverns; i++)
	{
		auto currentCavern = caverns.at(i);
		cout << "Cavern " << currentCavern->name << " coordinates: " << to_string(currentCavern->pos.x) << " " << to_string(currentCavern->pos.y) << endl;
	}
	cout << endl;

	// Connections
	for (int cavernNo = 0; cavernNo < noOfCaverns; cavernNo++)
	{
		auto currentCavern = caverns.at(cavernNo);
		for (int connectionNo = 0; connectionNo < noOfCaverns; connectionNo++)
		{
			// If there is a connection
			if (data.at(0) == 1)
			{
				currentCavern->connections.push_back(caverns.at(connectionNo));
			}

			data.erase(data.begin());
		}
	}

	// DEBUG
	for (int cavernNo = 0; cavernNo < noOfCaverns; cavernNo++)
	{
		auto currentCavern = caverns.at(cavernNo);
		for (auto connection : currentCavern->connections)
		{
			cout << "Cavern " << currentCavern->name << " connects to cave " << connection->name << endl;
		}
	}
	cout << endl;

	// Setup grid
	for (int x = 0; x < (int)biggestXCoord + 1; x++)
	{
		for (int y = 0; y < (int)biggestYCoord + 1; y++)
		{
			CircleShape newDot;
			newDot.setPosition(x * (WINDOW_WIDTH - margin * 2) / biggestXCoord + margin, y * (WINDOW_HEIGHT - margin * 2) / biggestYCoord + margin);
			newDot.setRadius(1.4f);
			newDot.setOrigin(0.7f, 0.7f);
			newDot.setFillColor(Color::Black);
			dots.push_back(newDot);
		}
	}

	// Setup circles for caves
	for (auto cavern : caverns)
	{
		cavern->shape.setPosition(cavern->pos.x * (WINDOW_WIDTH - margin * 2) / biggestXCoord + margin, cavern->pos.y *(WINDOW_HEIGHT - margin * 2) / biggestYCoord + margin);
		cavern->shape.setRadius(6.f);
		cavern->shape.setOrigin(3.f, 3.f);
		cavern->shape.setFillColor(Color::Black);

		// Different colour for first and last cavern
		if (cavern == startCavern)
		{
			cavern->shape.setFillColor(Color::Green);
		}
		if (cavern == endCavern)
		{
			cavern->shape.setFillColor(Color::Red);
		}
	}

	// Setup AI
	ai = AI();
	ai.openSet.push_back(startCavern);
	ai.currentNode = startCavern;
	//startCavern->setG(0);
	//startCavern->calcH();

}

float inputCooldown = 0;

void Update(RenderWindow &window)
{
	// Reset clock, recalculate deltatime
	static Clock clock;
	float dt = clock.restart().asSeconds();

	// Check and consume events
	Event event;
	while (window.pollEvent(event)) {
		if (event.type == Event::Closed) {
			window.close();
			return;
		}
	}

	// Quit via ESC Key
	if (Keyboard::isKeyPressed(Keyboard::Escape)) {
		window.close();
	}

	// Excecute ai
	if (!autoMode)
	{
		if (Keyboard::isKeyPressed(Keyboard::Space) && !ai.isFinished && inputCooldown < 0)
		{
			ai.nextStep();
			inputCooldown = 0.2f;
		}
	}
	else
	{
		while (!ai.isFinished)
		{
			ai.nextStep();
		}
	}

	// Change mode
	if (Keyboard::isKeyPressed(Keyboard::M) && inputCooldown < 0)
	{
		autoMode = !autoMode;
	}

	inputCooldown -= dt;

}

void Render(RenderWindow &window) {
	for (auto dot : dots)
	{
		window.draw(dot);
	}
	for (auto cavern : caverns)
	{
		window.draw(cavern->shape);
	}
}

int main()
{
	RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "AI coursework");
	Load();
	while (window.isOpen())
	{
		window.clear(Color::White);
		Update(window);
		Render(window);
		window.display();
	}

	return 0;
}