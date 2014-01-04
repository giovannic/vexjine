/*
 * AdaptiblePQueue.h: Priority queue that allows explicit updating of the underlying heap structure
 *
 *  Created on: 9 Apr 2010
 *      Author: nb605
 */

#ifndef ADAPTIBLEPQUEUE_H_
#define ADAPTIBLEPQUEUE_H_

#include <queue>
#include <vector>
#include <iterator>
#include <iostream>
using namespace std;

template<typename _Tp, typename _Sequence = vector<_Tp>, typename _Compare  = less<typename _Sequence::value_type> >
class AdaptiblePQueue : public std::priority_queue<_Tp, _Sequence, _Compare> {

public:
   void update() {
	   std::make_heap(this->c.begin(), this->c.end(), this->comp);
   }


   void print() {
	   cout << "Runnable threads list" << endl;
	   for (size_t i =0 ; i < this->c.size(); ++i) {
		   cout << "\t" << i << ") " << *(this->c[i]) << endl;
	   }
   }

   bool find(_Tp state) {
	   for (size_t i =0 ; i < this->c.size(); ++i) {
		   if (this->c[i] == state) {
			   return true;
		   }
	   }
	   return false;
   }

   typename _Sequence::iterator getQueueStart() {
	   return this->c.begin();
	}

   typename _Sequence::iterator getQueueEnd() {
	   return this->c.end();
	}

   void erase(_Tp state) {
	   typename _Sequence::iterator it = this->c.begin();
	   while (it != this->c.end()) {
		   if (*it == state) {
			   this->c.erase(it);
			   break;
		   }
		   ++it;
	   }
	   update();
   }
};

#endif /* ADAPTIBLEPQUEUE_H_ */
