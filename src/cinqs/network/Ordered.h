#ifndef NETWORK_ORDERED_H
#define NETWORK_ORDERED_H

class Customer;

class Ordered {
  virtual bool smallerThan( Customer *e ) = 0;
};

#endif
