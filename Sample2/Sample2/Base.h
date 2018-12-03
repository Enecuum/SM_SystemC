#pragma once

#include "stdafx.h"

extern double baseMu;
extern double baseSigma;
extern int baseWnum;
extern int baseSnum;
extern int baseAnum;
extern int t_proc;
extern int t_send;
extern int t_broadcast;

// positive only
// probably it was supposed to be exponential distribution? easy to change anyway...
int randomTime();
int randomTime(double mu, double sigma);
int randomTime(WnodeP w);
int randomTime(WnodeP w, k_block& k);
int randomTime(WnodeP w, m_block& m);
int randomTime(WSAP p, k_block& k);
int randomTime(SnodeP s, k_block& k);
int randomTime(SnodeP s, send_shadow_rq& srq);
int randomTime(SnodeP s, m_block& m);
int randomTime(SnodeP s, recv_shadow_rq& rr);
int randomTime(AnodeP a, k_block& k);
int randomTime(AnodeP a, m_block& m);
int randomTime(AnodeP a, recv_shadow_rq& rr);
int randomTime(AnodeP a, send_shadow_rq& srq);


struct write_if : virtual public sc_interface
{
	virtual void write(char) = 0;
	virtual void reset() = 0;
};

struct read_if : virtual public sc_interface
{
	virtual void read(char &) = 0;
	virtual int num_available() = 0;
};

struct fifo : public sc_channel, public write_if, public read_if
{
	int number;

	fifo(sc_module_name name, int size_, int num = 0) : sc_channel(name), size(size_)
	{
		number = num;
		data = new char[size];
		num_elements = first = 0;
		num_read = max_used = average = 0;
		last_time = SC_ZERO_TIME;
	}

	~fifo()
	{
		delete[] data;

		cout << endl << "Fifo " << this->name() << endl;
		cout << "Fifo size is: " << size << endl;
		cout << "Average fifo fill depth: " << double(average) / num_read << endl;
		cout << "Maximum fifo fill depth: " << max_used << endl;
		cout << "Average transfer time per character: " << last_time / num_read << endl;
		cout << "Total characters transferred: " << num_read << endl;
		cout << "Total time: " << last_time << endl;
	}

	void write(char c) {
		if (num_elements == size)
			wait(read_event);

		data[(first + num_elements) % size] = c;
		++num_elements;
		write_event.notify();
	}

	void read(char &c) {
		last_time = sc_time_stamp();
		if (num_elements == 0)
			wait(write_event);

		compute_stats();

		c = data[first];
		--num_elements;
		first = (first + 1) % size;
		read_event.notify();
	}

	void reset() { num_elements = first = 0; }

	int num_available() { return num_elements; }

	//private:
	char *data;
	int num_elements, first;
	sc_event write_event, read_event;
	int size, num_read, max_used, average;
	sc_time last_time;

	void compute_stats()
	{
		average += num_elements;

		if (num_elements > max_used)
			max_used = num_elements;

		++num_read;
	}
};
typedef fifo* fifoP;

