#pragma once

#include<list>
#include "wavread.h"

namespace Filter{
	class Generator{
		public:
			Generator();
			~Generator();

			virtual void fillBufferFloat(float *data, int len) = 0;
			virtual void fillBufferInt(int *data, int len) = 0;
			virtual void fillBufferU16(unsigned short *data, int len) = 0;
			//void fillBufferU8();
	};

	class Filter{
		protected:
			int _samplerate;
			void init(int samplerate){this->_samplerate = samplerate;}

			virtual void asdasd (int* inbuf, int* outbuf, int pos) = 0;

		public:
			Filter();
			virtual ~Filter();
	};

	class Chain : public Generator{
		private:
			int samplerate;
			float sampletime;

			Generator* generator;
			std::list<Filter*> filterChain;
			
			int bufsize;
			float *buffer[2];
			float *inbuf, *outbuf;

			void allocate(int l);
			void render(int l);

			inline void swapBuffers(){float*tmp = this->outbuf; this->inbuf = this->outbuf; this->outbuf = tmp;}

	public:
		Chain(int samplerate);
		virtual ~Chain();

		virtual void fillBufferFloat(float *data, int len);
		virtual void fillBufferInt(int *data, int len);
		virtual void fillBufferU16(unsigned short *data, int len);

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
		float* buffer;

	public:
		GeneratorWav(WavRead *reader); // : reader(reader){if (!reader) throw -5;}
		~GeneratorWav();

		virtual void fillBufferFloat(float *data, int len);
		virtual void fillBufferInt(int *data, int len);
		virtual void fillBufferU16(unsigned short *data, int len);
	}
};