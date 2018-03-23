#include <iostream>
#include <unordered_set>
#include <vector>
#include <queue>
#include <unordered_map>
#include "wikiscraper.h"
#include <algorithm>
#include <string>

using std::cout;             using std::endl;
using std::string;           using std::vector;
using std::unordered_set;    using std::unordered_map;
using std::priority_queue;   using std::priority_queue;

// function prototypes
vector<string> findWikiLadder(const string&, const string&);
int getCommonLinks(const vector<string>&, const unordered_set<string>&, WikiScraper&);
void printLadder(const vector<string>&);

/*
 * This function takes two strings representing the names of a start_page
 * and end_page and returns a ladder, represented as a vector<string>,
 * of links that can be followed from start_page to get to the end_page.
 */
vector<string> findWikiLadder(const string& start_page, const string& end_page) {

    // to keep track of links that have been visited
    unordered_set<string> visitedLinks;

    // create WikiScraper object
    WikiScraper scraper;
    auto target_set = scraper.getLinkSet(end_page);

    // lambda function for ladderPQ
    auto cmpFn = [&scraper,&target_set](const vector<string>& ladderA, const vector<string>& ladderB) -> bool {
        int num1 = getCommonLinks(ladderA,target_set,scraper);
        int num2 = getCommonLinks(ladderB,target_set,scraper);
        return num1 < num2;
    };

    // creates priority queue and enqueues firstLadder (start_page)
    priority_queue<vector<string>,vector<vector<string>>,decltype(cmpFn)> ladderPQ(cmpFn);
    vector<string> firstLadder;
    firstLadder.push_back(start_page);
    ladderPQ.push(firstLadder);

    while(!ladderPQ.empty()) {

        // dequeue highest priority ladder
        vector<string> topLadder = ladderPQ.top();
        ladderPQ.pop();
        printLadder(topLadder);
        string currentPage = topLadder.back();
        auto currentLinks = scraper.getLinkSet(currentPage);

        // check if end_page is in these links; if so, we are done!
        auto it = currentLinks.find(end_page);
        if (it!=currentLinks.end()) {
            topLadder.push_back(end_page);
            return topLadder;
        }

        for (const auto& link : currentLinks) {
            auto linkHasBeenVisited = visitedLinks.find(link);

            // if link was not found
            if (linkHasBeenVisited == visitedLinks.end()) {
                visitedLinks.insert(link);
                auto copyLadder = topLadder;
                copyLadder.push_back(link);
                ladderPQ.push(copyLadder);
            }
        }
    }
    return {};
}


// gets common links between last element of ladder and target set
int getCommonLinks(const vector<string>& ladder, const unordered_set<string>& target, WikiScraper& scraper) {
    unordered_set<string> setToReturn;
    string pageName = ladder.back();
    auto pageLinks = scraper.getLinkSet(pageName);
    for(auto it=pageLinks.begin(); it!=pageLinks.end(); ++it){
        if(target.find(*it)!=target.end()){
            setToReturn.insert(*it);
        }
    }
    return setToReturn.size();
}

// print partial ladders
void printLadder(const vector<string>& ladder) {
    for (string each: ladder) {
        cout << each << " --> ";
    }
    cout<<endl;
}


int main() {
    auto ladder = findWikiLadder("Milkshake", "Gene");
    if(ladder.empty()) {
        cout << "No ladder found!" << endl;
    } else {
        cout << "Ladder found:" << endl;
        cout << "\t";
        for (auto it=ladder.begin();it!=ladder.end();++it) {
            cout << *it << endl;
            cout << "\t";
        }
    }
    return 0;
}
