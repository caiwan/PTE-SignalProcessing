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
			virtual void fillBufferFloat(float *data, int len, int offset, WavRead::channel_t channel) = 0;
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

			virtual void reset() = 0;
			virtual void render(float* inbuf, float* outbuf, int pos) = 0;
	};

	class Chain : public Generator{
		private:
			int samplerate;
			float sampletime;

			Generator* generator;
			std::vector<Filter*> filterChain;
			
			int bufsize;
			float *buffer[2];
			float *inbuf, *outbuf;

			void allocate(int l);
			void render(int l, int o, WavRead::channel_t channel);

			inline void swapBuffers(){float*tmp = this->outbuf; this->inbuf = this->outbuf; this->outbuf = tmp;}

	public:
		Chain(int samplerate);
		virtual ~Chain();

		virtual void fillBufferFloat(float *data, int len = AUDIO_BUFFER_LEN, int offset = 0, WavRead::channel_t channel = WavRead::CH_MONO);
		virtual void fillBufferInt(int *data, int len = AUDIO_BUFFER_LEN, int offset = 0, WavRead::channel_t channel = WavRead::CH_MONO);
		virtual void fillBufferU16(unsigned short *data, int len = AUDIO_BUFFER_LEN, int offset = 0, WavRead::channel_t channel = WavRead::CH_MONO);

		inline void setGenerator(Generator *generator){this->generator = generator;}
		inline void addChain(Filter* filter){this->filterChain.push_back(filter);}
	};

	/////////////////////////////
	// Generatorok
	/////////////////////////////

	/// Csatolo wav file olvasohoz
	class GeneratorWav : public Generator{
		private:
			WavRead *reader;
			int bpos;
			float* buffer[2];

		public:
			GeneratorWav(WavRead *reader); // : reader(reader){if (!reader) throw -5;}
			~GeneratorWav();

		protected:
			virtual void fillBufferFloat(float *data, int len, int offset, WavRead::channel_t channel);
			//virtual void fillBufferInt(int *data, int len, WavRead::channel_t channel);
			//virtual void fillBufferU16(unsigned short *data, int len, WavRead::channel_t channel);
	};

	/// Negyszogjelet general
	class GanaratorPulse : public Generator{
	};
	
	/// Fureszt general
	class GanaratorSaw : public Generator{
		private:
			float freq;
			float sampling;
			//float falloff;
			//int lasttick;
		public:
			GanaratorSaw();
			GanaratorSaw(float freq, float sampling, float falloff);
			~GanaratorSaw();

			void tick(float freq, float sampling, float falloff){this->freq = freq, this->sampling = sampling;}

		protected:
			virtual void fillBufferFloat(float *data, int len, int offset, WavRead::channel_t channel);
	};
	
	/////////////////////////////
	// Filterek
	/////////////////////////////
	/// Finite Impulse Response -  Moving Average Konvolucios szuro
	/**
	
	*/
	class Filter_FFI_MA : public Filter{
		private:
			int M;
		public:
			Filter_FFI_MA(int m) : Filter() {
				this->M = m;
			}
			virtual void reset();
			virtual void render(float* inbuf, float* outbuf, int pos);
	};

	/// Finite Impulse Response -  Movin Average Konvolucios szuro
	/**
	
	*/
	class Filter_FFI_Sinc : public Filter{
		private:
			int M;
			float simga;
			float *impulse;


		public:
			Filter_FFI_Sinc(int m, float sigma);
			~Filter_FFI_Sinc();

			virtual void reset();
			virtual void render(float* inbuf, float* outbuf, int pos);
	};
};