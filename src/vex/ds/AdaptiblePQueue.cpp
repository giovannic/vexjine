/*
 * AdaptiblePQueue.cpp
 *
 *  Created on: 9 Apr 2010
 *      Author: nb605
 */

#include "AdaptiblePQueue.h"

struct intPtr_compare : binary_function<int *, int *, bool> {
	bool operator() (const int *lhs, const int *rhs) const {
		return (*lhs < *rhs);
	}
};

struct intPtr_compare2 : binary_function<int, int, bool> {
	bool operator() (const int lhs, const int rhs) const {
		return (lhs < rhs);
	}
};


/*
int main() {

	srand ( time(NULL) );

	AdaptiblePQueue<int, deque<int>, intPtr_compare2> bench;
	//AdaptiblePQueue<int, vector<int>, intPtr_compare2> bench;//NO COMPARISON!!!!
	int k, action;

	int insert =0 , del=0, heap = 0;
	for(int i =0 ; i< 10000; i++) {
		action = rand() % 11;
		if (action == 0) {
			bench.update();
			heap++;
		} else if (action > 5) {
			if (!bench.empty()) {
				bench.pop();
				del++;
			}
		} else {
			bench.push(rand() % 100);
			insert++;
		}
		if (i % 100 == 0) {
			cout << bench.size() << " " << bench.top() << endl;
		}
	}

	cout << "Inserted " << insert << " deleted " << del << " made heap " << heap << endl;

	AdaptiblePQueue<int *, vector<int *>, intPtr_compare> Q;

	//sleep(10);
	  int i1 = 4;
	  int i2 = 8;
	  int i3 = 212;
	  int i4 = 25;

	  Q.push(&i1);
	  Q.push(&i2);
	  Q.push(&i3);
	  Q.push(&i4);


	  //i3 = 2;

//	  Q.push(4);
//	  Q.push(2);
//	  Q.push(8);
//	  Q.push(5);
//	  Q.push(7);
//
//	  assert(Q.size() == 6);
//
//	  assert(Q.top() == 8);
//	  Q.pop();
//
//	  assert(Q.top() == 7);
//	  Q.pop();
//
//	  assert(Q.top() == 5);
//	  Q.pop();
//
//	  assert(Q.top() == 4);
//	  Q.pop();
//
//	  assert(Q.top() == 2);
//	  Q.pop();

	//  assert(Q.top() == 1);
	  //int i = Q.top();
	  //cout << i<<endl;
	  Q.print();

	  //Q.update();
	  int *itodel = &i2; Q.erase(itodel);

	  Q.print();
	  i2 = 100;
	  int *newblood1 = &i2;
	  Q.push(&i2);

	  Q.print();

	  itodel = &i3; Q.erase(itodel);

	  Q.print();
	  int *z= Q.top();
	  cout << *z << endl;
	  //assert(Q.empty());
	return 0;
}

*/
