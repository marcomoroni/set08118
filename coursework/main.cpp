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
	Node* parent;
};

class AI
{
public:
	AI() = default;
	vector<Node*> openSet;
	vector<Node*> closedSet;
	Node* currentNode;
	map<Node*, float> fs;
	map<Node*, float> gs;

	// Execute one step of A*
	void nextStep();

	bool isFinished = false;
};

AI ai;

int noOfCaverns;
vector<Node*> caverns;
Node* startCavern;
Node* endCavern;
vector<Node*> finalPath;

vector<CircleShape> dots; // Grid
CircleShape caveHighlight;
vector<RectangleShape> tunnels;

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
	cout << "Current node: " << currentNode->name << endl;
	if (!openSet.empty())
	{

		for (auto connection : currentNode->connections)
		{
			// if connected node is not in closed set
			if (!(find(closedSet.begin(), closedSet.end(), connection) != closedSet.end()))
			{
				openSet.push_back(connection);
			}
		}

		cout << "Open set:";
		for (auto c : openSet)
		{
			cout << " " << c->name;
		}
		cout << endl;

		cout << "Closed set:";
		for (auto c : closedSet)
		{
			cout << " " << c->name;
		}
		cout << endl;

		for (auto cavern : openSet)
		{
			if (cavern != currentNode)
			{
				// Calculate evaluation function
				float tempG = calculate_g(gs[currentNode], currentNode, cavern);
				float tempF = tempG + calculate_h(cavern, endCavern);
				cout << "Evaluation function for node " << cavern->name << ": " << to_string(tempF) << endl;

				// 
				if (fs.find(cavern) == fs.end() || tempF < fs[cavern])
				{
					gs[cavern] = tempG;
					fs[cavern] = tempF;

					if (find(currentNode->connections.begin(), currentNode->connections.end(), cavern) != currentNode->connections.end())
					{
						cavern->parent = currentNode;
					}
				}
			}
		}

		// Move current node from open set to closed set
		openSet.erase(remove(openSet.begin(), openSet.end(), currentNode), openSet.end());
		closedSet.push_back(currentNode);

		// Select new current node from open set with the lowest f
		Node* newCurrentNode = openSet.at(0);
		for (auto n : openSet)
		{
			if (fs[n] <= fs[newCurrentNode])
			{
				newCurrentNode = n;
			}
		}
		currentNode = newCurrentNode;

		// If end cavern reached
		if (currentNode == endCavern)
		{
			isFinished = true;
		}

		cout << endl;

	}
}

/*vector<Node*> calculate_final_path_from(Node* n)
{
	vector<Node*> path;

	path.push_back(n);
	if (n->parent != nullptr)
	{
		calculate_final_path_from(n->parent);
	}

	return path;
}*/

void reconstruct_final_path(Node* c)
{
	finalPath.push_back(c);
	if (c->parent != nullptr)
	{
		reconstruct_final_path(c->parent);
	}
}

void Reset()
{

}

void Load()
{
	string path = "res/input2.cav";

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

	// Draw connections

	// Setup grid
	for (int x = 0; x < (int)biggestXCoord + 1; x++)
	{
		for (int y = 0; y < (int)biggestYCoord + 1; y++)
		{
			CircleShape newDot;
			newDot.setPosition(x * (WINDOW_WIDTH - margin * 2) / biggestXCoord + margin, y * (WINDOW_HEIGHT - margin * 2) / biggestYCoord + margin);
			newDot.setRadius(1.4f);
			newDot.setOrigin(1.4f, 1.4f);
			newDot.setFillColor(Color::Black);
			dots.push_back(newDot);
		}
	}

	// Setup circles for caves
	for (auto cavern : caverns)
	{
		cavern->shape.setPosition(cavern->pos.x * (WINDOW_WIDTH - margin * 2) / biggestXCoord + margin, cavern->pos.y *(WINDOW_HEIGHT - margin * 2) / biggestYCoord + margin);
		cavern->shape.setRadius(6.f);
		cavern->shape.setOrigin(6.f, 6.f);
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

	// Setup visualisation of tunnels
	for (auto cavern : caverns)
	{
		for (auto connection : cavern->connections)
		{
			// Set dimentions
			float lenght = sqrt((cavern->shape.getPosition().x - connection->shape.getPosition().x) * (cavern->shape.getPosition().x - connection->shape.getPosition().x) + (cavern->shape.getPosition().y - connection->shape.getPosition().y) * (cavern->shape.getPosition().y - connection->shape.getPosition().y));
			RectangleShape line({lenght, 1.5f});
			line.setOrigin({ 0, 1.5f / 2 });
			// Set position
			line.setPosition(cavern->shape.getPosition());
			// Set rotation
			line.setRotation(atan2(connection->shape.getPosition().y - cavern->shape.getPosition().y, connection->shape.getPosition().x - cavern->shape.getPosition().x));
			// Set style
			line.setFillColor(Color::Black);

			tunnels.push_back(line);
		}
	}

	// Set up cave highlight
	caveHighlight.setRadius(20.f);
	caveHighlight.setOrigin(20.f, 20.f);
	caveHighlight.setFillColor(Color(0, 0, 0, 10));

	// Setup AI
	ai = AI();
	ai.openSet.push_back(startCavern);
	ai.currentNode = startCavern;
	ai.gs[startCavern] = 0.0f;
	ai.fs[startCavern] = 0.0f + calculate_h(startCavern, endCavern);

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

	// Final path
	if (ai.isFinished && finalPath.empty())
	{
		reconstruct_final_path(endCavern);

		cout << "Final path:";
		reverse(finalPath.begin(), finalPath.end());
		for (auto c : finalPath)
		{
			cout << " " << c->name;
		}
		cout << endl << endl;
	}

	// Change mode
	if (Keyboard::isKeyPressed(Keyboard::M) && inputCooldown < 0)
	{
		autoMode = !autoMode;
	}

	inputCooldown -= dt;

	caveHighlight.setPosition(ai.currentNode->shape.getPosition());

}

void Render(RenderWindow &window) {
	window.draw(caveHighlight);
	for (auto dot : dots)
	{
		window.draw(dot);
	}
	for (auto tunnel : tunnels)
	{
		window.draw(tunnel);
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