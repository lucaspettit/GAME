
#include "Observer.h"

using namespace std;

EventArgs::EventArgs(int id, string message) : Id(id), Message(message){}

string EventArgs::ToString() {
	string str = "[" + NAME + ", " + to_string(Id) + "] ";
	str += Message + "\n";
	return str;
}