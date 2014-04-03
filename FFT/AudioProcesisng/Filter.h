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
			virtual complex getFreq(float freq) = 0;
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

			void addFilter(Filter *filter);
			void calculate(complex* inbuf, complex *outbuf);
	};

	//
	// Kesz filterek
	//

	//------------
	class FilterLowPass : public Filter{
		protected:
			virtual complex getFreq(float freq);
			//void init(int freqstep, int bufsze);

		private:
			float cutFreq, falloff, phaseShift;

		public:
			FilterLowPass (float cutFreq, float falloff, float phaseShift);
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

