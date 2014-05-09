#pragma once

#include<vector>
#include "wavread.h"

namespace Filter{
	
	class Filter;
	class Chain;

	class Generator{
		public:
			Generator();
			~Generator();
		
		public:
			virtual void fillBufferFloat(double *data, int len, WavRead::channel_t channel) = 0;
			//virtual void fillBufferInt(int *data, int len, WavRead::channel_t channel) = 0;
			//virtual void fillBufferU16(unsigned short *data, int len, WavRead::channel_t channel) = 0;
			//void fillBufferU8();
	};

	class Filter{
		protected:
			int _samplerate;
			void init(int samplerate){this->_samplerate = samplerate;}

		public:
			Filter(){}
			virtual ~Filter(){}

			virtual void render(double* inbuf, double* outbuf, int pos) = 0;
	};

	class Chain : public Generator{
		private:
			int samplerate;
			double sampletime;

			Generator* generator;
			std::vector<Filter*> filterChain;
			
			int bufsize;
			double *buffer[2];
			double *inbuf, *outbuf;

			void allocate(int l);
			void render(int l, WavRead::channel_t channel);

			inline void swapBuffers(){double*tmp = this->outbuf; this->inbuf = this->outbuf; this->outbuf = tmp;}

	public:
		Chain(int samplerate);
		virtual ~Chain();

		virtual void fillBufferFloat(double *data, int len = AUDIO_BUFFER_LEN, WavRead::channel_t channel = WavRead::CH_MONO);
		virtual void fillBufferInt(int *data, int len = AUDIO_BUFFER_LEN, WavRead::channel_t channel = WavRead::CH_MONO);
		virtual void fillBufferU16(unsigned short *data, int len = AUDIO_BUFFER_LEN, WavRead::channel_t channel = WavRead::CH_MONO);

		inline void setGenerator(Generator *generator){this->generator = generator;}
		inline void addChain(Filter* filter){this->filterChain.push_back(filter);}
	};

	/////////////////////////////
	// Generator
	/////////////////////////////
	class GeneratorWav : public Generator{
		private:
			WavRead *reader;
			int bpos;
			double* buffer[2];

		public:
			GeneratorWav(WavRead *reader); // : reader(reader){if (!reader) throw -5;}
			~GeneratorWav();

		protected:
			virtual void fillBufferFloat(double *data, int len, WavRead::channel_t channel);
			//virtual void fillBufferInt(int *data, int len, WavRead::channel_t channel);
			//virtual void fillBufferU16(unsigned short *data, int len, WavRead::channel_t channel);
	};


	/////////////////////////////
	// Filterek
	/////////////////////////////

	class FilterRunningAvg : public Filter{
	private:
		int M;
	public:
		FilterRunningAvg(int m) : Filter() {
			this->M = m;
		}

		virtual void render(double* inbuf, double* outbuf, int pos);
	};
};