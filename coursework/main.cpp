#include <SFML\Graphics.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>

using namespace std;
using namespace sf;

class Cavern
{
public:
	Cavern() = default;
	Vector2i pos;
	vector<Cavern> connections = {};
};

int noOfCaverns;
vector<Cavern*> caverns;
Cavern* startCavern;
Cavern* endCavern;

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
		throw string("Couldn't open level file: ") + path;
	}

	// DEBUG
	cout << "Buffer: " << buffer << endl;

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
	for (int i = 0; i < 6; i++)
	{
		cout << data.at(i) << ",";
	}
	cout << "..." << endl;

	// Coordinates
	for (int i = 0; i < noOfCaverns; i++)
	{
		auto newCavern = new Cavern();
		newCavern->pos = { data.at(0), data.at(1) };
		data.erase(data.begin());
		data.erase(data.begin());
		caverns.push_back(newCavern);		
	}

	// DEBUG
	for (int i = 0; i < noOfCaverns; i++)
	{
		cout << "Cavern " << i << " coordinates: " << to_string(caverns.at(i)->pos.x) << " " << to_string(caverns.at(i)->pos.y) << endl;
	}
}

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
}

void Render(RenderWindow &window) {

}

int main()
{
	RenderWindow window(VideoMode(200, 200), "AI coursework");
	Load();
	while (window.isOpen())
	{
		window.clear();
		Update(window);
		Render(window);
		window.display();
	}

	return 0;
}