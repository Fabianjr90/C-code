#include <iostream>
#include "SimpleGraph.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <ctime>

using std::cout;	  using std::endl;
using std::string;    using std::cin;

//function prototypes
void Welcome();
void readGraph();
std::ifstream streamCreator();
int getInteger();
string GetLine();
std::vector<Node> nodeCreator(const int numNodes);
std::vector<Edge> edgeCreator(std::ifstream& myStream);
void calculateRepulsiveForces(const SimpleGraph& myGraph,
                              std::vector<double>& nodePosX,
                              std::vector<double>& nodePosY);
void calculateAttractiveForces(const SimpleGraph& myGraph,
                               std::vector<double>& nodePosX,
                               std::vector<double>& nodePosY);
void moveNodes(SimpleGraph& myGraph,
               std::vector<double>& nodePosX,
               std::vector<double>& nodePosY);

//main method
int main() {

    Welcome();

    //will re-prompt forever
    while (true) {

        //prompts user for file name and opens it
        cout << "Please enter a file name\n> ";
        std::ifstream myStream = streamCreator();

        cout << "Great! Now, please enter the number of seconds you'd like to iterate for\n> ";
        int numTotalSeconds = getInteger();

        //initialize graph
        SimpleGraph myGraph;

        //create nodes
        int kNumNodes;
        myStream >> kNumNodes;
        std::vector<Node> myNodes = nodeCreator(kNumNodes);
        myGraph.nodes = myNodes;

        //create edges
        std::vector<Edge> myEdges = edgeCreator(myStream);
        myGraph.edges = myEdges;

        InitGraphVisualizer(myGraph);
        DrawGraph(myGraph);

        //begin timer
        time_t startTime = time(NULL);
        double elapsedTime = 0.0;

        while(elapsedTime < double(numTotalSeconds)) {

            //make vectors of node position changes
            std::vector<double> nodeDelX(kNumNodes,0.0);
            std::vector<double> nodeDelY(kNumNodes,0.0);

            //calculate repulsive forces
            calculateRepulsiveForces(myGraph,nodeDelX,nodeDelY);

            //calculate attractive forces
            calculateAttractiveForces(myGraph,nodeDelX,nodeDelY);

            //node movement
            moveNodes(myGraph,nodeDelX,nodeDelY);

            //update graph
            DrawGraph(myGraph);

            //update time
            elapsedTime = difftime(time(NULL), startTime);
        }

        cout << "DONE!" << endl;
        cout << endl;
    }
    return 0;
}


/* Prints a message to the console welcoming the user and
 * describing the program.
 */
void Welcome() {
    cout << "Welcome to CS106L GraphViz!" << endl;
    cout << "This program uses a force-directed graph layout algorithm" << endl;
    cout << "to render sleek, snazzy pictures of various graphs." << endl;
    cout << endl;
}


/* This function prompts the user for a file name,
 * then opens the file and makes sure it's valid.
 * Will reprompt if the file does not exist
 * (similar to code found in course reader)
 */
std::ifstream streamCreator() {
    std::ifstream myStream;
    while(true) {
        string fileName;
        std::getline(cin,fileName);
        myStream.open(fileName);
        if (myStream) break;
        std::cerr << "Couldn't open the file!" << endl;
        cout << "Please enter a valid input file name\n> ";
    }
    return myStream;
}


/* Converts user input to an integer
 * Will reprompt if the input is not valid
 * (from course reader)
 */
int getInteger() {
    while(true) {
        std::stringstream converter;
        converter << GetLine();
        int myInt;
        if (converter >> myInt) {
            char remaining;
            if (converter >> remaining) {
                cout << "Unexpected character: " << remaining << endl;
            } else {
                return myInt;
            }
        } else {
            cout << "Please enter an integer." << endl;
        }
        cout << "Retry: ";
    }
}

/* Takes an input from the user (from class)
 */
string GetLine() {
    string result;
    std::getline(cin,result);
    return result;
}


/* Uses the number of nodes given to create Node,
 * places nodes along the unit circle, and
 * pushes Node structs into a vector<Node>
 */
std::vector<Node> nodeCreator(const int numNodes) {
    const double kPi = 3.14159265358979323;
    std::vector<Node> myVectorNode;
    for (int i=0; i<numNodes; ++i) {
        Node myNode;
        myNode.x = cos((2*kPi*double(i))/double(numNodes));
        myNode.y = sin((2*kPi*double(i))/double(numNodes));
        myVectorNode.push_back(myNode);
    }
    return myVectorNode;
}


/* Reads the input stream to create Edge structs
 * Pushes Edge structs into a vector<Edge>
 */
std::vector<Edge> edgeCreator(std::ifstream& myStream) {
    std::vector<Edge> myEdgeVector;
    int leftEdge;
    int rightEdge;
    while (myStream >> leftEdge >> rightEdge) {
        Edge myEdge;
        myEdge.start = leftEdge;
        myEdge.end = rightEdge;
        myEdgeVector.push_back(myEdge);
    }
    return myEdgeVector;
}


/* Calculates repulsive forces between nodes
 * Keeps track of delX and delY for each node
 */
void calculateRepulsiveForces(const SimpleGraph& myGraph,
                              std::vector<double>& nodePosX,
                              std::vector<double>& nodePosY) {
    double k_repel = 0.001;
    auto myNodes = myGraph.nodes;
    for (size_t i = 0; i<myNodes.size(); ++i) {
        for (size_t j = 0; j<myNodes.size(); ++j) {
            if (i==j) continue;

            double Frepel = k_repel / sqrt ( pow(myNodes[j].y-myNodes[i].y,2) +
                                           pow(myNodes[j].x-myNodes[i].x,2));

            double theta = atan2(myNodes[j].y-myNodes[i].y,
                                 myNodes[j].x-myNodes[i].x);

            nodePosX[i] -= Frepel * cos(theta);
            nodePosY[i] -= Frepel * sin(theta);
            nodePosX[j] += Frepel * cos(theta);
            nodePosY[j] += Frepel * sin(theta);
        }
    }
}


/* Calculates attractive forces between nodes
 * connected by edges. Keeps track of delX and
 * delY for each node
 */
void calculateAttractiveForces(const SimpleGraph& myGraph,
                               std::vector<double>& nodePosX,
                               std::vector<double>& nodePosY) {
    double k_attract = 0.001;
    auto myNodes = myGraph.nodes;
    auto myEdges = myGraph.edges;
    for (size_t i = 0; i < myEdges.size(); ++i) {
        Node startNode = myNodes[myEdges[i].start];
        Node endNode = myNodes[myEdges[i].end];

        double Fattract = k_attract *
                (pow(endNode.y-startNode.y,2) +
                 pow(endNode.x-startNode.x,2));

        double theta = atan2(endNode.y-startNode.y,endNode.x-startNode.x);

        nodePosX[myEdges[i].start] += Fattract * cos(theta);
        nodePosY[myEdges[i].start] += Fattract * sin(theta);
        nodePosX[myEdges[i].end] -= Fattract * cos(theta);
        nodePosY[myEdges[i].end] -= Fattract * sin(theta);
    }
}


/* Updates the positions of all nodes according
 * to the net forces (repulsive and attractive)
 */
void moveNodes(SimpleGraph& myGraph,
               std::vector<double>& nodePosX,
               std::vector<double>& nodePosY) {

    for (size_t i = 0; i < nodePosX.size(); ++i) {
        myGraph.nodes[i].x += nodePosX[i];
        myGraph.nodes[i].y += nodePosY[i];
    }
}
