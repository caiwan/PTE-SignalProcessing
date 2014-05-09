#pragma once

#include "FFT.h"
#include <vector>

/**
* // ezek a szurok nem idotaromanyban, hanem frekvencia tartomanyban dolgoznak
* tobbfele filtert lehet definialni, maiket ossze lehet fuzni egy filter chainbe
* 
*/
namespace FreqFilter{	
	class FilterChain;

	// absztrakt filter 
	class Filter{
		friend class FilterChain;
	
		protected:
			complex *freq_chr;
			int sampling;

		protected:
			// quick-and-dirty ganyolas a CPU-s textura generatorom alapjan
			virtual complex getFreq(double freq) = 0;
			void init(int sampling, int bufsze);
			inline const complex * getFqChar() {return this->freq_chr;}; // freq. karakterisztika

		public:
			Filter();
			virtual ~Filter(void);
	};

	// fq chain
	class FilterChain{
		private:
			std::vector<Filter *> filterList;
			int windowsize, samplerate;

		public:
			FilterChain(int windowsize, int samplerate);
			virtual ~FilterChain();

			inline void addFilter(Filter *filter){this->filterList.push_back(filter); filter->init(this->samplerate, this->windowsize); }
			void calculate(const complex* inbuf, complex *outbuf);
	};

	//
	// Kesz filterek
	//

	//------------
	class FilterLowPass : public Filter{
		protected:
			virtual complex getFreq(double freq);
			//void init(int freqstep, int bufsze);

		private:
			double cutFreq, falloff, phaseShift;

		public:
			FilterLowPass (double cutFreq, double falloff, double phaseShift);
			virtual ~FilterLowPass ();
	};

	//------------
	class FilterHighPass : public Filter{
	
	};

	//------------
	class FilterBandPass : public Filter{
	
	};

	//------------
	class FilterNotch : public Filter{
	
	};

};

