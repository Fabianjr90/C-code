/* Name: Fabian E. Ortega
 * This code uses four distinct ways of calculating the shortest path between a start 
 * node and end node. Definitions for "shortest" paths vary between algorithms.
 * Functions descriptions are found below.
 */

#include "Trailblazer.h"
#include "queue.h"
#include "map.h"
#include "pqueue.h"
#include "set.h"
#include <cfloat>

using namespace std;


//Prototypes of helper functions. Look how many I have!
double nodeCost(const RoadGraph& graph, const Path& path);

double nodeHeuristic(const RoadGraph& graph, RoadNode* current, RoadNode* end);

Vector<RoadEdge*> getEdges(const RoadGraph& graph, const Path& path);

bool isTwentyUnique(Path mainPath, Path altPath);

Path alternativeRouteEdge(const RoadGraph& graph, RoadNode* start, RoadNode* end,
                          RoadEdge* edgeToExclude);


/* This function uses the Breadth-First Search algorithm to find shortest path
 * from start node to end node. This algorithm has no knowledge of edge cost, so
 * it defines the "shortest" path as the path with elast number of hops
 */
Path breadthFirstSearch(const RoadGraph& graph, RoadNode* start, RoadNode* end) {

    //edge case
    if (start==end){
        return {start};
    }

    Queue<Path> myPaths;
    Map<RoadNode*,string> myNodeColors;

    //set node to yellow
    start->setColor(Color::YELLOW);
    myNodeColors[start] = "yellow";

    //create first path and enqueue start node
    Path firstPath;
    firstPath += start;
    myPaths.enqueue(firstPath);

    while(!myPaths.isEmpty()) {

        Path deqPath = myPaths.dequeue();
        RoadNode* lastNode = deqPath[deqPath.size()-1];
        lastNode->setColor(Color::GREEN);
        myNodeColors[lastNode] = "green";

        //test if we've reached our destination
        if (lastNode == end) {
            return deqPath;
        }

        //else, get node's neighbors and loop through them
        auto neighbors = graph.neighborsOf(lastNode);

        for (RoadNode* eachNode : neighbors) {

            //only if they are not present in our Color Map (AKA, gray nodes)
            if (myNodeColors.get(eachNode) == "") {

                //color neighbors yellow
                eachNode->setColor(Color::YELLOW);
                myNodeColors[eachNode] = "yellow";

                //enqueue new paths containing yellow nodes
                Path copyPath = deqPath;
                copyPath+=eachNode;
                myPaths.enqueue(copyPath);
            }
        }
    }
    return {};
}


/* This function uses the Dijkstra's algorithm to find shortest path
 * from start node to end node. This algorithm takes edge cost into account,
 * but has no knowledge of the overall shape of the map
 */
Path dijkstrasAlgorithm(const RoadGraph& graph, RoadNode* start, RoadNode* end) {

    //edge case
    if (start==end){
        return {start};
    }

    //use pqueue
    PriorityQueue<Path> myPaths;
    Map<RoadNode*,string> myNodeColors;

    //set node to yellow
    start->setColor(Color::YELLOW);
    myNodeColors[start] = "yellow";

    //create first path and enqueue start node
    Path firstPath;
    firstPath += start;
    myPaths.enqueue(firstPath,0.0);

    while(!myPaths.isEmpty()) {

        Path deqPath = myPaths.dequeue();
        RoadNode* lastNode = deqPath[deqPath.size()-1];

        if (lastNode == end) {
            lastNode->setColor(Color::GREEN);
            return deqPath;
        }

        if (myNodeColors.get(lastNode) != "green") {

            //set node to green
            lastNode->setColor(Color::GREEN);
            myNodeColors[lastNode] = "green";

            //get node's neighbors and loop through them
            auto neighbors = graph.neighborsOf(lastNode);

            for (RoadNode* eachNode : neighbors) {

                //only if they are not green
                if (myNodeColors.get(eachNode) != "green") {

                    //color neighbors yellow
                    eachNode->setColor(Color::YELLOW);
                    myNodeColors[eachNode] = "yellow";

                    //enqueue path using distance as priority
                    Path copyPath = deqPath;
                    copyPath+=eachNode;                    
                    myPaths.enqueue(copyPath,nodeCost(graph,copyPath));
                }
            }
        }
    }
    return {};
}


/* This function uses the A* Search algorithm to find shortest path
 * from start node to end node. This algorithm takes edge cost and map knowedledge
 * into account, and is therefore, faster than two previous algorithms
 */
Path aStar(const RoadGraph& graph, RoadNode* start, RoadNode* end) {

    //edge case
    if (start==end){
        return {start};
    }

    //use pqueue
    PriorityQueue<Path> myPaths;
    Map<RoadNode*,string> myNodeColors;

    //set node to yellow
    start->setColor(Color::YELLOW);
    myNodeColors[start] = "yellow";

    //create first path and enqueue start node
    Path firstPath;
    firstPath += start;
    myPaths.enqueue(firstPath,0.0);

    while(!myPaths.isEmpty()) {

        Path deqPath = myPaths.dequeue();
        RoadNode* lastNode = deqPath[deqPath.size()-1];

        if (lastNode == end) {
            lastNode->setColor(Color::GREEN);
            return deqPath;
        }

        if (myNodeColors.get(lastNode) != "green") {

            //set node to green
            lastNode->setColor(Color::GREEN);
            myNodeColors[lastNode] = "green";

            //get node's neighbors and loop through them
            auto neighbors = graph.neighborsOf(lastNode);

            for (RoadNode* eachNode : neighbors) {

                //only if they are not green
                if (myNodeColors.get(eachNode) != "green") {

                    //color neighbors yellow
                    eachNode->setColor(Color::YELLOW);
                    myNodeColors[eachNode] = "yellow";

                    //enqueue path using distance PLUS HEURISTIC as priority
                    Path copyPath = deqPath;
                    copyPath+=eachNode;
                    myPaths.enqueue(copyPath,
                                    nodeCost(graph,copyPath) +
                                    nodeHeuristic(graph,eachNode,end));
                }
            }
        }
    }
    return {};
}


/* This function uses the A* Search algorithm to find shortest/main path (described above).
 * To identify an alternate route, this function loops through each edge of the
 * main path and calls a revised form of the A* Search algorithm (named alternativeRouteEdge;
 * described below) that will exclude each of these edges. Out of these alternative paths,
 * only those that are at least 20% unique from the main path will be considered. Out of
 * these paths, the shortest is returned.
 */
Path alternativeRoute(const RoadGraph& graph, RoadNode* start, RoadNode* end) {

    //find best path (using A*)
    Path mainPath = aStar(graph,start,end);

    //get main path's edges
    Vector<RoadEdge*> myEdges = getEdges(graph,mainPath);

    //to save alternative paths (that are at least 20% unique)
    Vector<Path> altPaths;

    //loop through edges, excluding each of them
    for (RoadEdge* eachEdge : myEdges) {
        Path newAltPath = alternativeRouteEdge(graph,start,end,eachEdge);
        if (isTwentyUnique(mainPath,newAltPath)) {
            altPaths+=newAltPath;
        }
    }

    //if no alternative paths fit criteria (20% unique or more)
    if (altPaths.isEmpty()) return {};

    //else, find the shortest path among alternative paths
    double mySentinel = DBL_MAX;
    Path shortestAltPath;

    for (Path eachPath : altPaths) {
        if (nodeCost(graph,eachPath) < mySentinel) {
            shortestAltPath = eachPath;
            mySentinel = nodeCost(graph,eachPath);
        }
    }
    return shortestAltPath;
}


/* helper function for alternativeRoute function (see above). It uses the structure
 * of A* Search, but will exclude a specific edge (appropriately named "edgeToExclude")
 */
Path alternativeRouteEdge(const RoadGraph& graph, RoadNode* start, RoadNode* end,
                          RoadEdge* edgeToExclude) {

    //edge case
    if (start==end){
        return {start};
    }

    //use pqueue
    PriorityQueue<Path> myPaths;
    Map<RoadNode*,string> myNodeColors;

    //set node to yellow
    start->setColor(Color::YELLOW);
    myNodeColors[start] = "yellow";

    //create first path and enqueue start node
    Path firstPath;
    firstPath += start;
    myPaths.enqueue(firstPath,0.0);

    while(!myPaths.isEmpty()) {

        Path deqPath = myPaths.dequeue();
        RoadNode* lastNode = deqPath[deqPath.size()-1];

        if (lastNode == end) {
            lastNode->setColor(Color::GREEN);
            return deqPath;
        }

        if (myNodeColors.get(lastNode) != "green") {

            //set node to green
            lastNode->setColor(Color::GREEN);
            myNodeColors[lastNode] = "green";

            //get node's neighbors and loop through them
            auto neighbors = graph.neighborsOf(lastNode);

            for (RoadNode* eachNode : neighbors) {

                RoadEdge* currEdge = graph.edgeBetween(lastNode,eachNode);

                //only if they are not green
                if (myNodeColors.get(eachNode) != "green"
                        && currEdge->cost() != edgeToExclude->cost()) {

                    //color neighbors yellow
                    eachNode->setColor(Color::YELLOW);
                    myNodeColors[eachNode] = "yellow";

                    //enqueue path using distance + heuristic as priority
                    Path copyPath = deqPath;
                    copyPath+=eachNode;
                    myPaths.enqueue(copyPath,
                                    nodeCost(graph,copyPath) +
                                    nodeHeuristic(graph,eachNode,end));
                }
            }
        }
    }
    return {};
}


//get cost of new node for Dijkstra's algorithm
double nodeCost(const RoadGraph& graph, const Path& path) {
    Path deqPath = path;
    RoadNode* firstNode = deqPath[0];
    deqPath.remove(0);
    double dist = 0.0;
    for (RoadNode* eachNode : deqPath) {
        RoadEdge* myEdge = graph.edgeBetween(firstNode,eachNode);
        dist += myEdge->cost();
        firstNode = eachNode;
    }
    return dist;
}


//calculate Heuristic for A* Search algorithm
double nodeHeuristic(const RoadGraph& graph, RoadNode* current, RoadNode* end) {
    return graph.crowFlyDistanceBetween(current,end)/graph.maxRoadSpeed();
}


//gets all edges within a path
Vector<RoadEdge*> getEdges(const RoadGraph& graph, const Path& path) {

    Vector<RoadEdge*> myEdges;

    Path copyPath = path;
    RoadNode* firstNode = copyPath[0];
    copyPath.remove(0);

    for (RoadNode* eachNode : copyPath) {
        RoadEdge* mySingleEdge = graph.edgeBetween(firstNode,eachNode);
        myEdges+=mySingleEdge;
        firstNode = eachNode;
    }
    return myEdges;
}


/* determines whether alternative path is at least 20% unique
 * converts Path (Vector of nodes) into a Set of Nodes
 * to make the identification of unique nodes easier
 */
bool isTwentyUnique(Path mainPath, Path altPath) {

    Set<RoadNode*> mainPathSet;
    for (RoadNode* eachNode: mainPath) {
        mainPathSet+=eachNode;
    }

    Set<RoadNode*> altPathSet;
    for (RoadNode* eachNode2: altPath) {
        altPathSet+=eachNode2;
    }

    Set<RoadNode*> uniqueElems = altPathSet - mainPathSet;

    int numUniqueElems = uniqueElems.size();
    int numNodesAltPath = altPath.size();

    if (double(numUniqueElems)/double(numNodesAltPath) >= 0.20) {
        return true;
    } else {
        return false;
    }
}
