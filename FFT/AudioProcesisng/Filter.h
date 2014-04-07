#pragma once

#include<list>

namespace Filter{
	class Generator{
	};

	class Filter{
		protected:
			int _samplerate;
			void init(int samplerate){this->_samplerate = samplerate;}

			virtual void getAt(int* inbuf, int* outbuf, int pos) = 0;

		public:
			Filter();
			virtual ~Filter();
	};

	class Chain{
		private:
			Generator* generator;
			std::list<Filter*> filterChain;
			int *buffer[2];
			int *inbuf, *outbuf;

	public:
		Chain(int samplerate);
		virtual ~Chain();

		void calculate(void* buf, int length);
	};

};