/*
 Copyright (c) 2001 
 Author: Konstantin Boukreev 
 E-mail: konstantin@mail.primorye.ru 

 Created: 18.10.2001 15:59:17
 Version: 1.0.0

 feedforward multilayer neural network
 back-propagation algorithm
 
 example:
 -------------------------
 creates a net with input, hidded and ouput layers
 input - 10 synapses, 20 hidden neurons and 5 axons in output layer

 MLNet net(3, 10, 20, 5);	
 net.propagate(v_in, v_out); 

*/

#ifndef _MLNet_ee20b78a_a20f_44bd_abef_4a5a2e63504e
#define _MLNet_ee20b78a_a20f_44bd_abef_4a5a2e63504e

#if _MSC_VER > 1000 
#pragma once
#endif // _MSC_VER > 1000

class MLNet
{
public:
	typedef float value_t;
	typedef std::vector<value_t> array_t;
	typedef value_t (*fn_transfer)(value_t);
	
	class Learn
	{
	public:
		typedef value_t (*fn_derivative)(value_t);

	protected:
		MLNet&			m_net;
		array_t			m_errors;
		array_t			m_deltas;		
		fn_derivative	m_fd; 
		
	public:
		Learn(MLNet&, fn_derivative);
		virtual ~Learn() {}

		void epoch(array_t& v_in, array_t& v_ideal, double N, double M);
		void run  (unsigned cycles, double N0, double N1, double M);
		
	protected:
		virtual bool get(unsigned, unsigned, array_t& v_in, array_t& v_ideal) = 0;

		void compute_errors(array_t& v_out, array_t& v_ideal);
		void compute_deltas(array_t& v_out, double, double);
		void update();	
		void randomize_weigths(float, float);
		bool check_convergence(double);
	//	void check_paralysis(array_t&, double);

		void propagate(array_t& in_synapsis, array_t& out_axons);

		float delta (double N, double M, double d, double e , double y) 
		{		
			return  float( N * ((M * d) + ((1. - M) * e * y)));
		//	return  float(- N * ((M * d) + ((1. - M) * e * y)));
		}		
	};

	friend class MLNet::Learn;
	
	MLNet(unsigned number_of_layers, ...);

	fn_transfer set_transfer_function(fn_transfer fn)	{fn_transfer t = m_fn; m_fn = fn; return t;}
	void set_minmax(value_t min_p, value_t max_p)		{m_min = min_p; m_max = max_p; m_r = float(1. / m_max);}
	void set_bias(value_t bias)							{m_bias = bias;}
	value_t get_min() const								{return m_min;}
	value_t get_max() const								{return m_max;}
	unsigned get_input_size() const						{return m_layers.front();}
	unsigned get_output_size() const					{return m_layers.back();}

	// warning: will change a parameters of in_synapsys and out_axons vectors (size, capacity, values) 	
	void propagate(array_t& in_synapsis, array_t& out_axons);

	friend 
	std::ostream& operator << (std::ostream&, MLNet&);

	array_t::iterator begin_weight()			{return m_weights.begin();}
	array_t::iterator end_weight()				{return m_weights.end();}
	array_t::size_type size_weight()			{return m_weights.size();}	
	
 private:

	void propagate_layer(array_t::iterator i_begin, array_t::iterator i_end,
		array_t::iterator o_begin, array_t::iterator o_end,	array_t::iterator& w);

	typedef std::vector<unsigned> layers_t;

	array_t			m_weights;
	layers_t		m_layers;
	fn_transfer		m_fn;
	value_t			m_min;
	value_t			m_max;	
	value_t			m_r;		// reciprocal value : 1 / m_max		
	value_t			m_bias;
};

#endif //_MLNet_ee20b78a_a20f_44bd_abef_4a5a2e63504e

