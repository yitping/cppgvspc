//
//  gvspcFifo.h
//  cppgvspc
//
//  Created by Yitping Kok on 7/10/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#ifndef cppgvspc_gvspcFifo_h
#define cppgvspc_gvspcFifo_h

#include <deque>

template <typename T>
class gvspcFifo
{
	std::deque<T> q;
	int size_limit;

public:
	gvspcFifo();
	gvspcFifo(int s);
	~gvspcFifo();
	
	void limit(int s);
	void add(const T& qi);
	void clear();
	int size();
	int isEmpty();
	const T& operator[](int i) const;
	const T& last() const;
	T mean() const;
	T var() const;
	
};

template <typename T>
gvspcFifo<T>::gvspcFifo()
{
	std::cout << "empty gvspcFifo of type " << typeid(T).name() << " created." << std::endl;
}

template <typename T>
gvspcFifo<T>::gvspcFifo(int s)
{
	limit(s);
	std::cout << "1 gvspcFifo of type " << typeid(T).name() << " created." << std::endl;
}

template <typename T>
gvspcFifo<T>::~gvspcFifo()
{
	std::cout << "1 gvspcFifo of type " << typeid(T).name() << " destroyed." << std::endl;
}

template <typename T>
void gvspcFifo<T>::limit(int s) { size_limit = s; }

template <typename T>
void gvspcFifo<T>::add(const T& qi)
{
	if (q.size() == size_limit) { q.pop_front(); }
	q.push_back(qi);
	std::cout << "new size = " << q.size() << std::endl;
}

template <typename T>
void gvspcFifo<T>::clear() { q.clear(); }

template <typename T>
int gvspcFifo<T>::size() { return q.size(); }

template <typename T>
int gvspcFifo<T>::isEmpty() { return q.empty(); }

template <typename T>
const T& gvspcFifo<T>::operator[](int i) const { return q[i]; }

template <typename T>
const T& gvspcFifo<T>::last() const { return q.back(); }

template <typename T>
T gvspcFifo<T>::mean() const
{
	T mu;
	for (int i=0; i<q.size(); i++) mu = (i == 0) ? q[i] : mu + q[i];
	mu /= q.size();
	return mu;
}

template <typename T>
T gvspcFifo<T>::var() const
{
	T mu = mean();
	T var;
	for (int i=0; i<q.size(); i++) var = (i == 0) ? (q[i]-mu)*(q[i]-mu) : var + (q[i]-mu)*(q[i]-mu);
	var /= q.size()-1;
	return var;
}

#endif
